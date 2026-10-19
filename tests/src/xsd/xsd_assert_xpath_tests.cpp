#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Начало и конец схемы типизированных значений: fee — xs:decimal со
    ** значением по умолчанию, any — без типа (xs:anyType, нетипизированный),
    ** box — содержимое только из элементов; атрибуты flag (xs:boolean) и
    ** n (xs:double). Между ними вставляются утверждения теста.
    **
    ** The head and the tail of the typed values schema: fee is xs:decimal with
    ** a default value, any has no type (xs:anyType, untyped),
    ** box is element-only content; the attributes flag (xs:boolean) and
    ** n (xs:double). The test assertions are inserted between them.
    */
    const char* const TYPED_HEAD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"><xs:element name=\"p\"><xs:complexType>"
        "<xs:sequence><xs:element name=\"fee\" type=\"xs:decimal\" default=\"0.05\"/><xs:element name=\"any\"/>"
        "<xs:element name=\"box\"><xs:complexType><xs:sequence><xs:element name=\"v\" type=\"xs:int\"/>"
        "</xs:sequence></xs:complexType></xs:element></xs:sequence>"
        "<xs:attribute name=\"flag\" type=\"xs:boolean\"/><xs:attribute name=\"n\" type=\"xs:double\"/>";
    const char* const TYPED_TAIL = "</xs:complexType></xs:element></xs:schema>";

    /*
    ** Что видит утверждение: пустой элемент со значением по умолчанию —
    ** это значение (как в PSVI); нетипизированный текст "0.10" складывается
    ** точно; xs:boolean "1" равен true(); xs:double считается в double.
    ** У элемента только из элементов типизированного значения нет — ошибка.
    **
    ** What an assertion sees: an empty element with a default value is
    ** that value (as in the PSVI); an untyped text "0.10" is added
    ** exactly; xs:boolean "1" equals true(); xs:double is computed in double.
    ** An element-only element has no typed value — an error.
    */
    void testTypedValues()
    {
        std::string xml = "<p flag=\"1\" n=\"0.25\"><fee/><any>0.10</any><box><v>1</v></box></p>";
        std::string xsd = std::string(TYPED_HEAD) + "<xs:assert test=\"fee = 0.05\"/>"
            "<xs:assert test=\"any + 0.20 = 0.30\"/><xs:assert test=\"@flag = true()\"/>"
            "<xs:assert test=\"@n * 2 = 0.5 and string(fee) = '0.05'\"/>" + TYPED_TAIL;
        std::vector<beresta::Diagnostic> errors = support::agreedSchemaErrors(xsd, xml);

        TEST_ASSERT_EQ(support::describeDiagnostics(errors), "");
        xsd = std::string(TYPED_HEAD) + "<xs:assert test=\"box = 1\"/>" + TYPED_TAIL;
        errors = support::agreedSchemaErrors(xsd, xml);
        TEST_ASSERT(errors.size() == 1);
        TEST_ASSERT(errors[0].message.find("у элемента box с содержимым только из элементов") != std::string::npos);
    }

    /*
    ** Одно утверждение test над элементом t (атрибуты from / to типа xs:date,
    ** rate — xs:decimal, kind — строка; дети item). Возвращает ошибки,
    ** одинаковые во всех трёх режимах.
    **
    ** One assertion test over the element t (the attributes from / to of the type xs:date,
    ** rate — xs:decimal, kind — a string; the children item). Returns the errors
    ** equal in all three modes.
    */
    std::vector<beresta::Diagnostic> check(const std::string& test, const std::string& xml)
    {
        std::string xsd = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"><xs:element name=\"t\">"
            "<xs:complexType><xs:sequence><xs:element name=\"item\" type=\"xs:string\" minOccurs=\"0\""
            " maxOccurs=\"unbounded\"/></xs:sequence><xs:attribute name=\"from\" type=\"xs:date\"/>"
            "<xs:attribute name=\"to\" type=\"xs:date\"/><xs:attribute name=\"rate\" type=\"xs:decimal\"/>"
            "<xs:attribute name=\"kind\" type=\"xs:string\"/><xs:assert test=\"" + test + "\"/>"
            "</xs:complexType></xs:element></xs:schema>";

        return support::agreedSchemaErrors(xsd, xml);
    }

    /*
    ** Конструкции XPath 2.0 утверждений: if, some / every, сравнения значений,
    ** даты (с часовыми поясами), конструкторы xs:*, exists / empty, точные
    ** div, mod, floor, round. Ошибка вычисления (приведение строки к числу,
    ** деление на ноль, арифметика дат) — нарушение, а не «тихая» истина.
    **
    ** The XPath 2.0 constructs of assertions: if, some / every, value comparisons,
    ** dates (with time zones), xs:* constructors, exists / empty, exact
    ** div, mod, floor, round. An evaluation error (casting a string to a number,
    ** division by zero, date arithmetic) is a violation, not a "silent" truth.
    */
    void testXPath2Constructs()
    {
        std::string t = "<t from=\"2020-01-31\" to=\"2020-02-01\" rate=\"0.125\" kind=\"a\"><item>x</item><item>y</item></t>";

        TEST_ASSERT(check("if (@kind = 'a') then count(item) ge 2 else false()", t).empty());
        TEST_ASSERT(check("some $i in item satisfies $i eq 'y'", t).empty());
        TEST_ASSERT(check("every $i in item satisfies string-length($i) = 1", t).empty());
        TEST_ASSERT(check("@from lt @to and @to gt xs:date('2020-01-31')", t).empty());
        TEST_ASSERT(check("xs:dateTime('2020-01-01T10:00:00+03:00') eq xs:dateTime('2020-01-01T07:00:00Z')", t).empty());
        TEST_ASSERT(check("exists(@from) and empty(@missing) and not(exists(other))", t).empty());
        TEST_ASSERT(check("@rate * 100 = 12.5 and round(@rate * 100) = 13 and floor(-0.5) = -1", t).empty());
        TEST_ASSERT(check("10 div 4 = 2.5 and 7 mod 3 = 1 and -7 mod 3 = -1 and 1 div 3 * 3 lt 1", t).empty());
        TEST_ASSERT(check("xs:decimal('0.1') + xs:decimal('0.2') eq xs:decimal('0.3') and xs:integer('7') = 7", t).empty());
        TEST_ASSERT(check("some $i in item, $j in item satisfies $i != $j", t).empty());
        TEST_ASSERT(check("if (@kind = 'b') then true() else @rate gt 1", t).size() == 1);
        TEST_ASSERT(check("every $i in item satisfies $i = 'x'", t).size() == 1);
        std::vector<beresta::Diagnostic> errors = check("@kind + 1 = 2", t);
        TEST_ASSERT(errors.size() == 1 && errors[0].message.find("нельзя привести к типу xs:double") != std::string::npos);
        errors = check("1 div (@rate - 0.125) = 1", t);
        TEST_ASSERT(errors.size() == 1 && errors[0].message.find("деление на ноль") != std::string::npos);
        errors = check("@from + 1 = @to", t);
        TEST_ASSERT(errors.size() == 1 && errors[0].message.find("арифметика над датами") != std::string::npos);
        errors = check("item + 1 = 2", t);
        TEST_ASSERT(errors.size() == 1 && errors[0].message.find("узлов 2") != std::string::npos);
    }
}

namespace suites
{
    // Регистрация тестов языка утверждений (подмножество XPath 2.0)
    // Registers the tests of the assertion language (a subset of XPath 2.0)
    void registerXsdAssertXPathTests(test::TestRunner& runner)
    {
        runner.add("XSD 1.1: xs:assert, типизированные значения узлов", testTypedValues);
        runner.add("XSD 1.1: xs:assert, конструкции XPath 2.0", testXPath2Constructs);
    }
}
