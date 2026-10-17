#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

#include <string>

namespace
{
    // Строит строку из depth вложенных элементов <n><n>...</n></n>
    std::string buildNested(std::size_t depth)
    {
        std::string xml;

        for (std::size_t index = 0; index < depth; ++index)
            xml += "<n>";
        for (std::size_t index = 0; index < depth; ++index)
            xml += "</n>";
        return xml;
    }

    // Глубина в пределах лимита разбирается, сверх лимита — понятная ошибка
    void testMaxDepth()
    {
        beresta::ParserOptions options;
        options.maxDepth = 50;
        beresta::XmlParser parser(options);

        TEST_ASSERT(parser.parse(buildNested(50)).getRoot() != nullptr);
        TEST_ASSERT_THROWS(parser.parse(buildNested(51)), beresta::ParseException);
    }

    // parseElement возвращает автономный корень без родителя
    void testParseElementFragment()
    {
        std::unique_ptr<beresta::Element> element =
            beresta::XmlParser().parseElement("<?xml version=\"1.0\"?><x a='1'><y/></x>");

        TEST_ASSERT(element->getParent() == nullptr);
        TEST_ASSERT_EQ(element->toString(), "<x a=\"1\"><y/></x>");
    }

    // Пробелы внутри тегов допустимы в любых местах, где их разрешает XML
    void testWhitespaceInsideTags()
    {
        const char* input = "<a\n\tx\n=\n'1'\t\ty = \"2\"\n><b\n/></a\n>";

        TEST_ASSERT_EQ(support::roundTrip(input), "<a x=\"1\" y=\"2\"><b/></a>");
    }
}

namespace suites
{
    // Регистрация тестов ограничений и граничных случаев
    void registerLimitTests(test::TestRunner& runner)
    {
        runner.add("границы: максимальная глубина", testMaxDepth);
        runner.add("границы: разбор фрагмента", testParseElementFragment);
        runner.add("границы: пробелы внутри тегов", testWhitespaceInsideTags);
    }
}
