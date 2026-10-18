#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "i18n/i18n.hpp"

#include <set>

namespace
{
    // Номера меток {N} в шаблоне (метки в двойных скобках {{N}} — тоже метки)
    // Placeholder numbers {N} in a template (placeholders inside {{N}} count too)
    std::set<char> placeholders(const char* text)
    {
        std::set<char> found;
        std::string_view view(text);

        for (std::size_t index = 0; index + 2 < view.size(); ++index)
        {
            if (view[index] == '{' && view[index + 1] >= '0' && view[index + 1] <= '9' && view[index + 2] == '}')
                found.insert(view[index + 1]);
        }
        return found;
    }

    /*
    ** Полнота каталогов: у каждого сообщения есть русский и английский текст,
    ** и наборы меток {N} совпадают — иначе в одном из языков пропал бы аргумент.
    **
    ** Catalog completeness: every message has Russian and English text, and the
    ** sets of {N} placeholders match — otherwise an argument would vanish in a language.
    */
    void testCatalogsComplete()
    {
        for (std::size_t id = 0; id < static_cast<std::size_t>(beresta::i18n::Message::COUNT); ++id)
        {
            beresta::i18n::Message message = static_cast<beresta::i18n::Message>(id);
            const char* russian = beresta::i18n::lookup(beresta::i18n::RUSSIAN_CATALOG, message);
            const char* english = beresta::i18n::lookup(beresta::i18n::ENGLISH_CATALOG, message);
            TEST_ASSERT(std::string_view(russian) != "(message without text)");
            TEST_ASSERT(std::string_view(english) != "(message without text)");
            TEST_ASSERT(placeholders(russian) == placeholders(english));
        }
        std::vector<std::string_view> languages = beresta::i18n::availableLanguages();
        TEST_ASSERT(languages.size() >= 2 && languages[0] == "ru" && languages[1] == "en");
    }

    /*
    ** Смена языка во время работы: сообщения разбора, исключения DOM и
    ** XPath сразу приходят на английском; неизвестный код языка отвергается.
    **
    ** Switching the language at run time: parser messages, DOM exceptions and
    ** XPath errors come in English at once; an unknown language code is rejected.
    */
    void testSwitchLanguage()
    {
        TEST_ASSERT(beresta::i18n::setLanguage("en"));
        TEST_ASSERT_EQ(std::string(beresta::i18n::language()), "en");
        try
        {
            beresta::XmlParser().parse("<a><b></a>");
            TEST_ASSERT(false);
        }
        catch (const beresta::ParseException& error)
        {
            TEST_ASSERT_EQ(error.getMessage(), "line 1, column 7: expected </b>, found </a>");
        }
        TEST_ASSERT_THROWS(beresta::Element("1bad"), beresta::DomException);
        TEST_ASSERT(!beresta::i18n::setLanguage("xx"));
        TEST_ASSERT_EQ(std::string(beresta::i18n::language()), "en");
        TEST_ASSERT(beresta::i18n::setLanguage("ru"));
    }

    // Подстановка: порядок меток свободный, {{N}} даёт фигурные скобки, лишняя метка видна
    // Substitution: placeholders in any order, {{N}} yields braces, a missing argument stays visible
    void testFormatting()
    {
        TEST_ASSERT_EQ(beresta::i18n::format(beresta::i18n::Message::SCHEMA_ELEMENT_UNDECLARED, {"urn:x", "a"}),
            "элемент {urn:x}a не объявлен");
        TEST_ASSERT_EQ(beresta::i18n::format(beresta::i18n::Message::END_TAG_MISMATCH, {"b"}),
            "ожидался </b>, найден </{1}>");
        beresta::i18n::setLanguage("en");
        TEST_ASSERT_EQ(beresta::i18n::format(beresta::i18n::Message::AT_LINE_COLUMN, {"3", "5", "x"}),
            "line 3, column 5: x");
        beresta::i18n::setLanguage("ru");
    }
}

namespace suites
{
    // Регистрация тестов языков сообщений / Registration of message language tests
    void registerI18nTests(test::TestRunner& runner)
    {
        runner.add("i18n: полнота каталогов ru/en", testCatalogsComplete);
        runner.add("i18n: смена языка во время работы", testSwitchLanguage);
        runner.add("i18n: подстановка аргументов", testFormatting);
    }
}
