#include "suites/test_suites.hpp"
#include "support/parse_helpers.hpp"
#include "utils/seeded_hash.hpp"

#include <string>

namespace
{
    /*
    ** Тег с count атрибутами a0 … a(count-1); prefixed — каждый атрибут
    ** с префиксом p, связанным с одним URI. Атрибутов больше порога
    ** перебора, поэтому повторы ищутся хеш-таблицей.
    **
    ** A tag with count attributes a0 … a(count-1); prefixed — every attribute
    ** with the prefix p bound to one URI. There are more attributes than the loop
    ** threshold, so repetitions are searched by a hash table.
    */
    std::string manyAttributes(std::size_t count, bool prefixed)
    {
        std::string xml = "<r xmlns:p='urn:p' xmlns:q='urn:p'";

        for (std::size_t index = 0; index < count; ++index)
        {
            xml += " ";
            if (prefixed)
                xml += "p:";
            xml += "a" + std::to_string(index) + "='" + std::to_string(index) + "'";
        }
        return xml;
    }

    // Повтор имени после порога перебора по-прежнему находится; без повтора всё разбирается
    // A repeated name after the loop threshold is still found; without a repetition everything is parsed
    void testRepeatedAttributesBeyondThreshold()
    {
        beresta::Document document = beresta::XmlParser().parse(manyAttributes(2000, false) + "/>");

        TEST_ASSERT(document.getRoot()->getAttributes().size() == 2002);
        TEST_ASSERT_EQ(document.getRoot()->getAttribute("a1999").value_or(""), "1999");
        TEST_ASSERT_THROWS(beresta::XmlParser().parse(manyAttributes(2000, false) + " a7='x'/>"),
            beresta::ParseException);
        TEST_ASSERT(support::errorCodeOf(manyAttributes(2000, true) + " q:a7='x'/>")
            == beresta::DomErrorCode::NAMESPACE_ERROR);
    }

    // Пакетная установка: повтор заменяет значение, неверное имя не меняет элемент
    // Batch setting: a repetition replaces the value, an invalid name does not change the element
    void testBatchAttributes()
    {
        beresta::Element element("e");
        std::vector<beresta::Attribute> many;

        element.setAttribute("keep", "1");
        for (int index = 0; index < 40; ++index)
            many.push_back(beresta::Attribute{"k" + std::to_string(index % 20), std::to_string(index)});
        many.push_back(beresta::Attribute{"keep", "2"});
        element.setAttributes(many);
        TEST_ASSERT(element.getAttributes().size() == 21);
        TEST_ASSERT_EQ(element.getAttribute("k3").value_or(""), "23");
        TEST_ASSERT_EQ(element.getAttribute("keep").value_or(""), "2");
        TEST_ASSERT_THROWS(element.setAttributes({beresta::Attribute{"ok", "1"}, beresta::Attribute{"1bad", "2"}}),
            beresta::DomException);
        TEST_ASSERT(element.getAttributes().size() == 21);
        TEST_ASSERT(!element.hasAttribute("ok"));
    }

    // Хеш с ключом стабилен в пределах процесса и различает близкие строки
    // The keyed hash is stable within a process and tells close strings apart
    void testSeededHash()
    {
        beresta::utils::SeededHash hash;

        TEST_ASSERT(hash("item-12345678") == hash(std::string("item-12345678")));
        TEST_ASSERT(hash("item-12345678") != hash("item-12345679"));
        TEST_ASSERT(hash("") != hash(std::string(1, '\0')));
        TEST_ASSERT(hash("abcdefgh") != hash("abcdefg"));
    }
}

namespace suites
{
    // Регистрация тестов защиты от атак через коллизии и квадратичные проверки
    // Registers the tests of protection from collision and quadratic-check attacks
    void registerFloodTests(test::TestRunner& runner)
    {
        runner.add("Защита: повторы атрибутов за порогом перебора", testRepeatedAttributesBeyondThreshold);
        runner.add("Защита: пакетная установка атрибутов", testBatchAttributes);
        runner.add("Защита: хеш с секретным ключом", testSeededHash);
    }
}
