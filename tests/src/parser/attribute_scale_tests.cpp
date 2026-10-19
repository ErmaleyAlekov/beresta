#include "suites/test_suites.hpp"

#include "beresta.hpp"

#include <algorithm>
#include <chrono>
#include <string>

namespace
{
    // Пустой тег с count разными атрибутами a0='0' … / An empty tag with count distinct attributes a0='0' …
    std::string tagWith(std::size_t count)
    {
        std::string xml = "<r";

        xml.reserve(count * 16);
        for (std::size_t index = 0; index < count; ++index)
            xml += " a" + std::to_string(index) + "='" + std::to_string(index) + "'";
        return xml + "/>";
    }

    // Лучшее из пяти времён разбора в секундах (меньше всего шума планировщика)
    // The best of five parse times in seconds (the least scheduler noise)
    double parseSeconds(const std::string& xml)
    {
        double best = 1e9;

        for (int run = 0; run < 5; ++run)
        {
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            beresta::Document document = beresta::XmlParser().parse(xml);
            std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
            best = std::min(best, elapsed.count());
        }
        return best;
    }

    /*
    ** Проверка повторов атрибутов при разборе: до порога — перебор, дальше —
    ** хеш-таблица с SipHash-1-3 и секретным ключом процесса. Атакующий не
    ** может подобрать коллизии, поэтому время разбора растёт линейно.
    ** Сравнивается не абсолютное время (оно зависит от машины и санитайзеров),
    ** а рост: в восемь раз больше атрибутов — линейно ~8 раз, квадратично ~64.
    ** Порог 24 оставляет тройной запас на шум общих машин CI (macOS под
    ** Rosetta, санитайзеры) и всё равно ловит квадратичный рост.
    **
    ** The check for repeated attributes during parsing: up to the threshold — a scan,
    ** beyond it — a hash table with SipHash-1-3 and a secret process key. An attacker
    ** cannot pick collisions, so the parse time grows linearly.
    ** Not the absolute time is compared (it depends on the machine and the sanitizers),
    ** but the growth: eight times more attributes — linearly ~8 times, quadratically ~64.
    ** The threshold 24 leaves a threefold margin for the noise of shared CI machines
    ** (macOS under Rosetta, sanitizers) and still catches a quadratic growth.
    */
    void testAttributeParsingIsLinear()
    {
        double small = parseSeconds(tagWith(25000));
        double large = parseSeconds(tagWith(200000));

        TEST_ASSERT(large < 24.0 * std::max(small, 0.002));
    }

    /*
    ** Тег с 50 000 атрибутами: все на месте и по порядку, поиск по имени
    ** находит первый, средний и последний, XPath видит их все, а повтор
    ** имени в самом конце списка — ошибка разбора. Поиск по имени
    ** (getAttribute) у элемента — перебор, как в libxml2, Xerces и pugixml:
    ** для массового чтения есть getAttributes() и XPath.
    **
    ** A tag with 50,000 attributes: all are in place and in order, a lookup by
    ** name finds the first, the middle and the last one, XPath sees them all, and a
    ** repeated name at the very end of the list is a parse error. A lookup by name
    ** (getAttribute) of an element is a scan, as in libxml2, Xerces and pugixml:
    ** for bulk reading there are getAttributes() and XPath.
    */
    void testAttributeAccessAtScale()
    {
        std::string xml = tagWith(50000);
        beresta::Document document = beresta::XmlParser().parse(xml);
        const beresta::Element& root = *document.getRoot();
        std::size_t inOrder = 0;

        for (std::size_t index = 0; index < root.getAttributes().size(); ++index)
        {
            if (root.getAttributes()[index].value == std::to_string(index))
                ++inOrder;
        }
        TEST_ASSERT(inOrder == 50000);
        TEST_ASSERT_EQ(root.getAttribute("a0").value_or(""), "0");
        TEST_ASSERT_EQ(root.getAttribute("a25000").value_or(""), "25000");
        TEST_ASSERT_EQ(root.getAttribute("a49999").value_or(""), "49999");
        TEST_ASSERT(!root.hasAttribute("a50000"));
        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, "count(/r/@*)"), "50000");
        xml.insert(xml.size() - 2, " a49999='x'");
        TEST_ASSERT_THROWS(beresta::XmlParser().parse(xml), beresta::ParseException);
    }
}

namespace suites
{
    // Регистрация тестов масштабируемости атрибутов
    // Registers the attribute scalability tests
    void registerAttributeScaleTests(test::TestRunner& runner)
    {
        runner.add("Разбор: 25 000 и 200 000 атрибутов — линейный рост", testAttributeParsingIsLinear);
        runner.add("Разбор: доступ к 50 000 атрибутам", testAttributeAccessAtScale);
    }
}
