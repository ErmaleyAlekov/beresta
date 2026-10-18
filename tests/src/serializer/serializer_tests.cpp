#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Отступы расставляются только между элементами; смешанный текст — дословно
    // Indentation is placed only between elements; mixed text is written verbatim
    void testPrettyPrint()
    {
        beresta::Document document = beresta::XmlParser().parse(
            "<?xml version=\"1.0\"?><r><a><b/></a><p>Текст <i>курсив</i>!</p></r>");
        beresta::SerializerOptions options;
        options.prettyPrint = true;
        options.indent = "  ";

        std::string pretty = beresta::XmlSerializer(options).toString(document);
        TEST_ASSERT_EQ(pretty, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<r>\n  <a>\n    <b/>\n  </a>\n"
            "  <p>Текст <i>курсив</i>!</p>\n</r>\n");
        TEST_ASSERT(beresta::XmlParser().parse(pretty).getRoot()->isEqualNode(*document.getRoot()));
    }

    // Вывод в windows-1251: правильная декларация, кириллица одним байтом, эмодзи — ссылкой
    // Output in windows-1251: a correct declaration, Cyrillic in one byte, emoji as a reference
    void testEncodedOutput()
    {
        beresta::Element element("t");
        element.setTextContent("Да\xF0\x9F\x98\x80");
        beresta::SerializerOptions options;
        options.encoding = "cp1251";

        std::string bytes = beresta::XmlSerializer(options).toString(element);
        TEST_ASSERT_EQ(bytes, "<?xml version=\"1.0\" encoding=\"windows-1251\"?><t>\xC4\xE0&#128512;</t>");
        beresta::Document back = beresta::XmlParser().parse(bytes);
        TEST_ASSERT_EQ(back.getRoot()->getTextContent(), "Да\xF0\x9F\x98\x80");
    }

    // Файл в UTF-16 с BOM читается обратно без потерь
    // A file in UTF-16 with a BOM is read back without loss
    void testFileRoundTrip()
    {
        std::filesystem::path directory = support::makeTestDirectory("serializer");
        beresta::Document document = beresta::XmlParser().parse("<книга автор=\"Пушкин\">Онегин</книга>");
        beresta::SerializerOptions options;
        options.encoding = "UTF-16";

        beresta::XmlSerializer(options).writeFile(document, directory / "out.xml");
        beresta::Document back = beresta::XmlParser().parseFile(directory / "out.xml");
        TEST_ASSERT(back.getRoot()->isEqualNode(*document.getRoot()));
        TEST_ASSERT_EQ(back.getDeclaration()->encoding, "UTF-16");
    }
}

namespace suites
{
    // Регистрация тестов сериализатора
    // Registers the serializer tests
    void registerSerializerTests(test::TestRunner& runner)
    {
        runner.add("сериализатор: форматированный вывод", testPrettyPrint);
        runner.add("сериализатор: вывод в windows-1251", testEncodedOutput);
        runner.add("сериализатор: файл в UTF-16", testFileRoundTrip);
    }
}
