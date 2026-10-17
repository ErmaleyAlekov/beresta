#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Разбор файла с обработкой XInclude из каталога root
    std::string includeResult(const std::filesystem::path& root, const char* file)
    {
        beresta::FileResourceResolver resolver(root);
        beresta::ParserOptions options;
        options.resolver = &resolver;
        options.processXInclude = true;

        return beresta::XmlParser(options).parseFile(root / file).toString();
    }

    // Включение XML, текста и фрагмента по xpointer; вложенные включения
    void testIncludes()
    {
        std::filesystem::path root = support::makeTestDirectory("xinclude");
        const char* xi = "xmlns:xi=\"http://www.w3.org/2001/XInclude\"";
        support::writeTestFile(root / "chapter.xml", std::string("<chapter ") + xi
            + "><title>Глава</title><xi:include href=\"note.txt\" parse=\"text\"/></chapter>");
        support::writeTestFile(root / "note.txt", "заметка & текст");
        support::writeTestFile(root / "list.xml", "<list><a/><b>второй</b></list>");
        support::writeTestFile(root / "book.xml", std::string("<book ") + xi + "><xi:include href=\"chapter.xml\"/>"
            "<xi:include href=\"list.xml\" xpointer=\"element(/1/2)\"/></book>");

        TEST_ASSERT_EQ(includeResult(root, "book.xml"), "<book xmlns:xi=\"http://www.w3.org/2001/XInclude\">"
            "<chapter xmlns:xi=\"http://www.w3.org/2001/XInclude\"><title>Глава</title>заметка &amp; текст</chapter>"
            "<b>второй</b></book>");
    }

    // Недоступный ресурс заменяется содержимым xi:fallback
    void testFallback()
    {
        std::filesystem::path root = support::makeTestDirectory("xinclude_fallback");
        support::writeTestFile(root / "main.xml", "<r xmlns:xi=\"http://www.w3.org/2001/XInclude\">"
            "<xi:include href=\"missing.xml\"><xi:fallback><p>нет файла</p></xi:fallback></xi:include></r>");

        TEST_ASSERT_EQ(includeResult(root, "main.xml"),
            "<r xmlns:xi=\"http://www.w3.org/2001/XInclude\"><p>нет файла</p></r>");
    }

    // Циклическое включение и отсутствие резолвера — понятные ошибки
    void testIncludeErrors()
    {
        std::filesystem::path root = support::makeTestDirectory("xinclude_cycle");
        support::writeTestFile(root / "a.xml", "<a xmlns:xi=\"http://www.w3.org/2001/XInclude\">"
            "<xi:include href=\"b.xml\"/></a>");
        support::writeTestFile(root / "b.xml", "<b xmlns:xi=\"http://www.w3.org/2001/XInclude\">"
            "<xi:include href=\"a.xml\"/></b>");
        beresta::ParserOptions options;
        options.processXInclude = true;

        TEST_ASSERT_THROWS(includeResult(root, "a.xml"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::XmlParser(options).parse("<r xmlns:xi=\"http://www.w3.org/2001/XInclude\">"
            "<xi:include href=\"x.xml\"/></r>"), beresta::DomException);
    }
}

namespace suites
{
    // Регистрация тестов XInclude
    void registerXIncludeTests(test::TestRunner& runner)
    {
        runner.add("XInclude: XML, текст, xpointer", testIncludes);
        runner.add("XInclude: fallback", testFallback);
        runner.add("XInclude: ошибки", testIncludeErrors);
    }
}
