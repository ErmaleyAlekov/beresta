#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Эталонный вывод: монолитный, без единого лишнего пробельного символа
    // The reference output: monolithic, without a single extra whitespace character
    const char* const EXPECTED =
        "<?xml version=\"1.0\"?><catalog><item id=\"1\" type=\"a\"><name>Кофе</name>"
        "<price>10</price></item><item id=\"2\"/></catalog>";

    // Красиво отформатированный ввод: отступы пробелами и табами, переносы строк
    // Pretty-formatted input: indentation with spaces and tabs, line breaks
    void testPrettyInput()
    {
        const char* pretty =
            "<?xml version=\"1.0\"   ?>\r\n"
            "<catalog>\n"
            "    <item id=\"1\"\n"
            "\t\ttype='a'>\n"
            "        <name>Кофе</name>\n"
            "\t\t<price>10</price>\n"
            "    </item>\n"
            "    <item id=\"2\" />\n"
            "</catalog>\n\n";

        TEST_ASSERT_EQ(support::roundTrip(pretty), EXPECTED);
    }

    // Сырой ввод «в одну строку»: теги идут вплотную друг к другу
    // Raw "one-line" input: the tags go right next to each other
    void testCompactInput()
    {
        const char* compact =
            "<?xml version=\"1.0\"?><catalog><item id=\"1\" type=\"a\"><name>Кофе</name>"
            "<price>10</price></item><item id=\"2\"/></catalog>";

        TEST_ASSERT_EQ(support::roundTrip(compact), EXPECTED);
    }

    // Повторный разбор собственного вывода даёт тот же самый текст
    // Parsing its own output again gives exactly the same text
    void testOutputIsStable()
    {
        std::string once = support::roundTrip(EXPECTED);

        TEST_ASSERT_EQ(support::roundTrip(once), once);
    }

    // Текст с содержимым сохраняется дословно, отступы — только по опции
    // Text with content is kept verbatim, indentation only by the option
    void testWhitespaceOption()
    {
        const char* input = "<p>\n  <b> bold </b> tail \n</p>";
        beresta::ParserOptions options;
        options.preserveWhitespaceText = true;

        TEST_ASSERT_EQ(support::roundTrip(input), "<p><b> bold </b> tail \n</p>");
        TEST_ASSERT_EQ(beresta::XmlParser(options).parse(input).toString(),
            "<p>\n  <b> bold </b> tail \n</p>");
    }
}

namespace suites
{
    // Регистрация тестов независимости от форматирования
    // Registers the formatting independence tests
    void registerFormatTests(test::TestRunner& runner)
    {
        runner.add("формат: отформатированный ввод", testPrettyInput);
        runner.add("формат: ввод в одну строку", testCompactInput);
        runner.add("формат: стабильность вывода", testOutputIsStable);
        runner.add("формат: опция сохранения пробелов", testWhitespaceOption);
    }
}
