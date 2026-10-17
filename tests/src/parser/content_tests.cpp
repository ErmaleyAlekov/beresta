#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Пять стандартных сущностей и символьные ссылки (десятичные и hex)
    void testEntitiesAndCharacterReferences()
    {
        beresta::Document document = beresta::XmlParser().parse(
            "<t a=\"&quot;&apos;\">&lt;&gt;&amp; &#65;&#x42; &#1046; &#x1F600;</t>");
        const beresta::Element& root = *document.getRoot();

        TEST_ASSERT_EQ(root.getAttribute("a").value(), "\"'");
        TEST_ASSERT_EQ(root.getTextContent(), "<>& AB Ж \xF0\x9F\x98\x80");
    }

    // Комментарии, CDATA и инструкции обработки переживают круговой разбор
    void testMarkupRoundTrip()
    {
        const char* xml = "<!--top--><?app run?><r><!-- in --><![CDATA[<b>&</b>]]><?x?></r>";

        TEST_ASSERT_EQ(support::roundTrip(xml), xml);
    }

    // Смешанное содержимое: текст между тегами сохраняется на своих местах
    void testMixedContent()
    {
        const char* xml = "<p>Hello, <b>big</b> world<i>!</i></p>";

        TEST_ASSERT_EQ(support::roundTrip(xml), xml);
    }

    // Нормализация: \r\n -> \n в тексте; \t и \n -> пробел в атрибутах; BOM пропускается
    void testNormalizationAndBom()
    {
        beresta::Document document = beresta::XmlParser().parse(
            "\xEF\xBB\xBF<t a=\"x\ty\nz\">line1\r\nline2\rline3</t>");
        const beresta::Element& root = *document.getRoot();

        TEST_ASSERT_EQ(root.getAttribute("a").value(), "x y z");
        TEST_ASSERT_EQ(root.getTextContent(), "line1\nline2\nline3");
    }
}

namespace suites
{
    // Регистрация тестов содержимого
    void registerContentTests(test::TestRunner& runner)
    {
        runner.add("содержимое: сущности и символьные ссылки", testEntitiesAndCharacterReferences);
        runner.add("содержимое: комментарии, CDATA, PI", testMarkupRoundTrip);
        runner.add("содержимое: смешанный текст", testMixedContent);
        runner.add("содержимое: нормализация и BOM", testNormalizationAndBom);
    }
}
