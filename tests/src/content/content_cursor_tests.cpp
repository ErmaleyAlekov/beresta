#include "suites/test_suites.hpp"
#include "content/content_cursor.hpp"
#include "content/content_matcher.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace
{
    // Детерминированный генератор (линейный конгруэнтный): тест повторяем на любой платформе
    // A deterministic generator (linear congruential): the test is repeatable on any platform
    struct Generator
    {
        std::uint64_t state;

        std::uint32_t next()
        {
            state = state * 6364136223846793005ULL + 1442695040888963407ULL;
            return static_cast<std::uint32_t>(state >> 33);
        }
    };

    /*
    ** Случайная модель глубиной до трёх уровней: элементы a/b/c, xs:any,
    ** sequence, choice, all с повторами {0..3, unbounded}. Такие модели
    ** нарочно бывают неоднозначными — пошаговый автомат обязан совпасть с
    ** общим алгоритмом и на них.
    **
    ** A random model up to three levels deep: elements a/b/c, xs:any,
    ** sequence, choice, all with repetitions {0..3, unbounded}. Such models
    ** are deliberately ambiguous at times — the step-by-step automaton must agree with
    ** the general algorithm on them too.
    */
    beresta::content::Particle randomModel(Generator& random, int depth)
    {
        static const std::size_t OCCURS[] = {0, 1, 1, 1, 2, 3, beresta::content::UNBOUNDED};
        beresta::content::Particle particle;
        std::uint32_t kind = random.next() % 6;

        if (depth >= 3)
            kind = (random.next() % 2) * 5;
        if (kind == 0 || kind == 4)
        {
            particle.kind = beresta::content::ParticleKind::ELEMENT;
            particle.name = std::string(1, static_cast<char>('a' + random.next() % 3));
        }
        else if (kind == 5)
            particle.kind = beresta::content::ParticleKind::WILDCARD;
        else
            particle.kind = static_cast<beresta::content::ParticleKind>(kind);
        for (std::uint32_t count = random.next() % 4; kind >= 1 && kind <= 3 && count > 0; --count)
            particle.children.push_back(randomModel(random, depth + 1));
        particle.minOccurs = OCCURS[random.next() % 4];
        particle.maxOccurs = OCCURS[random.next() % 7];
        if (particle.maxOccurs < particle.minOccurs)
            particle.maxOccurs = particle.minOccurs;
        return particle;
    }

    // Итог курсора способом plan над всем списком детей
    // The cursor result with the plan plan over the whole child list
    beresta::content::MatchResult runCursor(const beresta::content::Particle& model,
        beresta::content::CursorPlan plan, const std::vector<beresta::content::ChildKey>& children)
    {
        beresta::content::ContentCursor cursor;

        cursor.reset(model, plan);
        for (const beresta::content::ChildKey& child : children)
            cursor.step(child);
        return cursor.finish();
    }

    /*
    ** Пять тысяч случайных моделей и последовательностей детей: автомат,
    ** буфер и выбранный для модели способ дают тот же итог и то же место
    ** ошибки, что и matchContent над готовым списком.
    **
    ** Five thousand random models and child sequences: the automaton,
    ** the buffer and the plan chosen for the model give the same result and the same error
    ** location as matchContent over a ready list.
    */
    void testCursorMatchesGeneralAlgorithm()
    {
        static const char* const NAMES[] = {"a", "b", "c"};
        Generator random{20261018};
        std::size_t mismatches = 0;

        for (int iteration = 0; iteration < 5000; ++iteration)
        {
            beresta::content::Particle model = randomModel(random, 0);
            std::vector<beresta::content::ChildKey> children;
            for (std::uint32_t count = random.next() % 10; count > 0; --count)
            {
                const char* name = NAMES[random.next() % 3];
                children.push_back(beresta::content::ChildKey{name, name, ""});
            }
            beresta::content::MatchResult expected = beresta::content::matchContent(model, children);
            const beresta::content::CursorPlan plans[] = {beresta::content::CursorPlan::AUTOMATON,
                beresta::content::CursorPlan::BUFFERED, beresta::content::choosePlan(model)};
            for (beresta::content::CursorPlan plan : plans)
            {
                beresta::content::MatchResult actual = runCursor(model, plan, children);
                if (actual.matched != expected.matched || (!actual.matched && actual.failIndex != expected.failIndex))
                    ++mismatches;
            }
        }
        TEST_ASSERT(mismatches == 0);
    }
}

namespace suites
{
    // Регистрация тестов пошаговой проверки модели содержимого
    // Registers the tests of step-by-step content model checking
    void registerContentCursorTests(test::TestRunner& runner)
    {
        runner.add("Модель содержимого: пошаговый курсор = общий алгоритм", testCursorMatchesGeneralAlgorithm);
    }
}
