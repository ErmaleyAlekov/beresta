#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Внутренние сущности: простой текст, текст с разметкой и вложенные ссылки
    // Internal entities: plain text, text with markup and nested references
    void testInternalEntities()
    {
        const char* xml =
            "<!DOCTYPE r [<!ENTITY who \"мир\"><!ENTITY hello \"Привет, &who;!\">"
            "<!ENTITY bold \"<b>&who;</b>\">]><r a=\"&hello;\">&hello; &bold;</r>";
        beresta::Document document = beresta::XmlParser().parse(xml);
        const beresta::Element* root = document.getRoot();

        TEST_ASSERT_EQ(root->getAttribute("a").value(), "Привет, мир!");
        TEST_ASSERT_EQ(root->getTextContent(), "Привет, мир! мир");
        TEST_ASSERT(root->findChildElement("b") != nullptr);
    }

    // DOCTYPE сохраняется при сериализации без лишних пробелов
    // The DOCTYPE is kept on serialization without extra spaces
    void testDoctypeRoundTrip()
    {
        const char* xml = "<!DOCTYPE note SYSTEM \"note.dtd\"[<!ENTITY a \"x\">]><note/>";
        beresta::Document document = beresta::XmlParser().parse(xml);

        TEST_ASSERT_EQ(document.toString(), xml);
        TEST_ASSERT_EQ(document.getDocumentType()->getSystemId(), "note.dtd");
    }

    // Атрибуты по умолчанию из ATTLIST и нормализация не-CDATA значений
    // Default attributes from ATTLIST and normalization of non-CDATA values
    void testAttributeDefaults()
    {
        const char* xml =
            "<!DOCTYPE r [<!ATTLIST r lang CDATA \"ru\" kind (a|b) #FIXED \"a\" tok NMTOKENS #IMPLIED>]>"
            "<r tok=\"  x   y \"/>";
        beresta::Document document = beresta::XmlParser().parse(xml);
        const beresta::Element* root = document.getRoot();

        TEST_ASSERT_EQ(root->getAttribute("lang").value(), "ru");
        TEST_ASSERT_EQ(root->getAttribute("kind").value(), "a");
        TEST_ASSERT_EQ(root->getAttribute("tok").value(), "x y");
    }

    // Рекурсия и «billion laughs» останавливаются с понятными ошибками
    // Recursion and "billion laughs" are stopped with clear errors
    void testEntityProtection()
    {
        beresta::ParserOptions options;
        options.maxEntityExpansions = 100;
        beresta::XmlParser parser(options);
        const char* recursive = "<!DOCTYPE r [<!ENTITY a \"&b;\"><!ENTITY b \"&a;\">]><r>&a;</r>";
        const char* laughs =
            "<!DOCTYPE r [<!ENTITY l \"ha\"><!ENTITY l2 \"&l;&l;&l;&l;&l;&l;&l;&l;&l;&l;\">"
            "<!ENTITY l3 \"&l2;&l2;&l2;&l2;&l2;&l2;&l2;&l2;&l2;&l2;\">"
            "<!ENTITY l4 \"&l3;&l3;&l3;&l3;&l3;&l3;&l3;&l3;&l3;&l3;\">]><r>&l4;</r>";

        TEST_ASSERT_THROWS(parser.parse(recursive), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse(laughs), beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<!DOCTYPE r [<!ENTITY e \"<a>\">]><r>&e;</r>"),
            beresta::ParseException);

        // Два раскрытия по два байта точно заполняют бюджет, третье превышает его.
        // Two two-byte expansions exactly fill the budget, and the third exceeds it.
        options.maxExpandedSize = 4;
        const beresta::XmlParser smallBudget(options);
        const std::string dtd = "<!DOCTYPE r [<!ENTITY a \"xy\">]>";
        TEST_ASSERT_EQ(smallBudget.parse(dtd + "<r>&a;&a;</r>").getRoot()->getTextContent(), "xyxy");
        try
        {
            smallBudget.parse(dtd + "<r>&a;&a;&a;</r>");
            TEST_ASSERT(false);
        }
        catch (const beresta::ParseException& error)
        {
            TEST_ASSERT(error.getCode() == beresta::DomErrorCode::LIMIT_EXCEEDED);
        }
    }
}

namespace suites
{
    // Регистрация тестов сущностей и DOCTYPE
    // Registers the entity and DOCTYPE tests
    void registerDtdEntityTests(test::TestRunner& runner)
    {
        runner.add("DTD: внутренние сущности", testInternalEntities);
        runner.add("DTD: DOCTYPE при сериализации", testDoctypeRoundTrip);
        runner.add("DTD: атрибуты по умолчанию", testAttributeDefaults);
        runner.add("DTD: защита от рекурсии и бомб", testEntityProtection);
    }
}
