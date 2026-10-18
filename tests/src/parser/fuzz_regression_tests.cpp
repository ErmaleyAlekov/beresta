#include "suites/test_suites.hpp"
#include "support/parse_helpers.hpp"

/*
** Регрессионные тесты находок фаззинга (tests/fuzz). Каждый тест — это
** минимизированный вход, на котором раньше срабатывал санитайзер или
** оракул фаззера «сериализованный документ обязан разбираться снова».
**
** Regression tests for fuzzing findings (tests/fuzz). Each test is a
** minimized input on which a sanitizer or the fuzzer oracle "a serialized
** document must parse again" used to fire.
*/
namespace
{
    /*
    ** fuzz_parser, оракул повторного разбора: инструкция <?xmlA ...?> без
    ** пробела после "xml" — это PI, а не XML-декларация, но детектор
    ** кодировки брал из неё encoding="iso-8859-1". В итоге документ БЕЗ
    ** декларации читался как Latin-1, а его же сериализация — как UTF-8.
    **
    ** fuzz_parser, the reparse oracle: the instruction <?xmlA ...?> without
    ** a space after "xml" is a PI, not an XML declaration, but the encoding
    ** detector took encoding="iso-8859-1" from it. As a result a document WITHOUT
    ** a declaration was read as Latin-1, and its own serialization as UTF-8.
    */
    void testInstructionDoesNotDeclareEncoding()
    {
        std::string latin = "<?xmlA encoding=\"iso-8859-1\"?><a>\xE9</a>";
        std::string utf8 = "<?xmlA encoding=\"iso-8859-1\"?><a>\xC3\xA9</a>";
        beresta::Document document = beresta::XmlParser().parse(utf8);
        std::string serialized = beresta::XmlSerializer().toString(document);

        TEST_ASSERT(support::errorCodeOf(latin) == beresta::DomErrorCode::ENCODING_ERROR);
        TEST_ASSERT_EQ(document.getRoot()->getTextContent(), "\xC3\xA9");
        TEST_ASSERT_EQ(beresta::XmlParser().parse(serialized).getRoot()->getTextContent(), "\xC3\xA9");
        TEST_ASSERT_EQ(beresta::XmlParser().parse("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?><a>\xE9</a>")
            .getRoot()->getTextContent(), "\xC3\xA9");
    }
}

namespace suites
{
    // Регистрация регрессионных тестов фаззинга
    // Registers the fuzzing regression tests
    void registerFuzzRegressionTests(test::TestRunner& runner)
    {
        runner.add("фаззинг: <?xmlA ...?> не задаёт кодировку", testInstructionDoesNotDeclareEncoding);
    }
}
