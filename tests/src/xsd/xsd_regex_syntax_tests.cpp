#include "suites/test_suites.hpp"
#include "exception/dom_exception.hpp"
#include "xsd/regex/xsd_regex.hpp"

namespace
{
    /*
    ** После '\' XSD 1.0 допускает только SingleCharEsc, \s \d \w \i \c (и их
    ** прописные дополнения) и \p{..} / \P{..}. Расширения Perl/.NET (\x41,
    ** A, \A, \b, \1) — ошибка схемы, а не буква после слэша.
    **
    ** After '\' XSD 1.0 allows only SingleCharEsc, \s \d \w \i \c (and their
    ** upper-case complements) and \p{..} / \P{..}. Perl/.NET extensions (\x41,
    ** A, \A, \b, \1) are a schema error, not the letter after the slash.
    */
    void testStrictEscapes()
    {
        TEST_ASSERT(beresta::xsd::Regex("\\n\\r\\t\\\\\\|\\.\\?\\*\\+\\(\\)\\{\\}\\-\\[\\]\\^").matches("\n\r\t\\|.?*+(){}-[]^"));
        TEST_ASSERT(beresta::xsd::Regex("foo\\s+bar").matches("foo \t bar"));
        TEST_ASSERT_THROWS(beresta::xsd::Regex("[\\x41]"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("\\u0041"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("\\Afoo"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("\\bfoo\\b"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("(a)\\1"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("\\$"), beresta::DomException);
    }

    /*
    ** Классы символов по грамматике XSD 1.0: диапазон, начало которого больше
    ** конца ([z-a], [a-;], [a-\\]), раньше молча переворачивался; пустой класс
    ** [] и '[' без экранирования внутри класса ([a[:xyz:]) раньше принимались.
    ** Теперь всё это — ошибки схемы, а \[ и вычитание -[...] работают как прежде.
    **
    ** Character classes by the XSD 1.0 grammar: a range whose start is greater than
    ** its end ([z-a], [a-;], [a-\\]) used to be silently reversed; an empty class
    ** [] and an unescaped '[' inside a class ([a[:xyz:]) used to be accepted.
    ** Now all of them are schema errors, while \[ and the subtraction -[...] work as before.
    */
    void testClassSyntax()
    {
        TEST_ASSERT(beresta::xsd::Regex("[a-a]").matches("a"));
        TEST_ASSERT(beresta::xsd::Regex("[\\[a]").matches("["));
        TEST_ASSERT(beresta::xsd::Regex("[a-z-[aeiou]]").matches("b"));
        TEST_ASSERT_THROWS(beresta::xsd::Regex("[z-a]"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("[a-;]"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("[a-\\\\]"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("a[]b"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("[^]"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Regex("[a[:xyz:]"), beresta::DomException);
    }

    /*
    ** Средние варианты выбора из трёх и более ветвей достижимы. MSVC 14.44 /O2 собирал прежний
    ** compileAlternation так, что «bb|cc|dd» не принимал «cc», а шаблон цвета Грани UI — «#172033».
    **
    ** The middle alternatives of a choice with three or more branches are reachable. MSVC 14.44 /O2
    ** built the former compileAlternation so that "bb|cc|dd" rejected "cc", and the Gran UI color pattern — "#172033".
    */
    void testLongAlternation()
    {
        beresta::xsd::Regex pairs("bb|cc|dd|ee");
        beresta::xsd::Regex color("#([0-9a-fA-F]{3}|[0-9a-fA-F]{4}|[0-9a-fA-F]{6}|[0-9a-fA-F]{8})");

        TEST_ASSERT(pairs.matches("bb") && pairs.matches("cc") && pairs.matches("dd") && pairs.matches("ee"));
        TEST_ASSERT(!pairs.matches("bc"));
        TEST_ASSERT(color.matches("#fff") && color.matches("#ffff") && color.matches("#172033") && color.matches("#17203380"));
        TEST_ASSERT(!color.matches("#12345"));
        TEST_ASSERT(beresta::xsd::Regex("a{3}|a{6}|a{8}").matches("aaaaaa"));
    }
}

namespace suites
{
    // Регистрация тестов строгого синтаксиса регулярных выражений XSD
    // Registers the tests of the strict XSD regular expression syntax
    void registerXsdRegexSyntaxTests(test::TestRunner& runner)
    {
        runner.add("XSD regex: только экранирования XSD 1.0", testStrictEscapes);
        runner.add("XSD regex: синтаксис классов символов", testClassSyntax);
        runner.add("XSD regex: средние ветви длинного выбора", testLongAlternation);
    }
}
