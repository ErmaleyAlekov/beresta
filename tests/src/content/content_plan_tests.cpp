#include "suites/test_suites.hpp"
#include "content/content_cursor.hpp"

namespace
{
    // Способ по форме модели: простая последовательность, общий случай, вложенный счётчик
    // The plan by the model shape: a simple sequence, the general case, a nested counter
    void testPlanChoice()
    {
        beresta::content::Particle sequence;
        beresta::content::Particle leaf;

        leaf.kind = beresta::content::ParticleKind::ELEMENT;
        leaf.name = "a";
        sequence.children.push_back(leaf);
        TEST_ASSERT(beresta::content::choosePlan(sequence) == beresta::content::CursorPlan::GREEDY_SEQUENCE);
        sequence.children.push_back(leaf);
        TEST_ASSERT(beresta::content::choosePlan(sequence) == beresta::content::CursorPlan::AUTOMATON);
        sequence.maxOccurs = 3;
        sequence.children[0].maxOccurs = 2;
        TEST_ASSERT(beresta::content::choosePlan(sequence) == beresta::content::CursorPlan::BUFFERED);
    }
}

namespace suites
{
    // Регистрация тестов выбора способа проверки модели
    // Registers the tests of choosing the model checking plan
    void registerContentPlanTests(test::TestRunner& runner)
    {
        runner.add("Модель содержимого: выбор способа проверки", testPlanChoice);
    }
}
