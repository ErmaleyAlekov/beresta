#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "support/parse_helpers.hpp"

namespace
{
    // DTD библиотеки: модель содержимого, ID/IDREF, обязательный атрибут
    // The library DTD: a content model, ID/IDREF, a required attribute
    const char* const LIBRARY_DTD =
        "<!DOCTYPE library [<!ELEMENT library (book+)><!ELEMENT book (title, author*)>"
        "<!ELEMENT title (#PCDATA)><!ELEMENT author (#PCDATA)>"
        "<!ATTLIST book id ID #REQUIRED ref IDREF #IMPLIED>]>";

    // Корректный документ проходит без ошибок
    // A valid document passes without errors
    void testValidDocument()
    {
        TEST_ASSERT(support::countValidityErrors(std::string(LIBRARY_DTD) + "<library><book id=\"b1\"><title>T</title>"
            "<author>A</author></book><book id=\"b2\" ref=\"b1\"><title>U</title></book></library>") == 0);
    }

    // Все ошибки собираются за один проход: модель, ID, IDREF, атрибут, элемент
    // All errors are collected in one pass: the model, ID, IDREF, an attribute, an element
    void testCollectsAllErrors()
    {
        std::size_t errors = support::countValidityErrors(std::string(LIBRARY_DTD) + "<library><book id=\"b1\"><author>A</author>"
            "</book><book id=\"b1\" ref=\"none\"><title>T</title><extra/></book>"
            "<book><title>X</title></book></library>");

        TEST_ASSERT(errors >= 6);
    }

    // Без ErrorHandler первая ошибка валидности прерывает разбор
    // Without an ErrorHandler the first validity error stops parsing
    void testFailFastWithoutHandler()
    {
        beresta::ParserOptions options;
        options.validateDtd = true;
        beresta::XmlParser parser(options);

        TEST_ASSERT_THROWS(parser.parse(std::string(LIBRARY_DTD) + "<library/>"),
            beresta::ParseException);
        TEST_ASSERT_THROWS(parser.parse("<library/>"), beresta::ParseException);
    }

    // getElementById работает по атрибутам, объявленным в DTD как ID, и по xml:id
    // getElementById works by the attributes declared as ID in the DTD and by xml:id
    void testGetElementById()
    {
        beresta::Document document = beresta::XmlParser().parse(std::string(LIBRARY_DTD)
            + "<library><book id=\"b7\"><title xml:id=\"t\">T</title></book></library>");

        TEST_ASSERT(document.getElementById("b7") != nullptr);
        TEST_ASSERT_EQ(document.getElementById("t")->getName(), "title");
        TEST_ASSERT(document.getElementById("nope") == nullptr);
    }
}

namespace suites
{
    // Регистрация тестов проверки по DTD
    // Registers the DTD validation tests
    void registerDtdValidationTests(test::TestRunner& runner)
    {
        runner.add("DTD: корректный документ", testValidDocument);
        runner.add("DTD: сбор всех ошибок", testCollectsAllErrors);
        runner.add("DTD: остановка без обработчика", testFailFastWithoutHandler);
        runner.add("DTD: getElementById", testGetElementById);
    }
}
