#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Наследование утверждений: Range требует low le high; Window расширяет
    ** Range (добавляет width le high - low), Positive ограничивает его
    ** (добавляет low ge 0); Money — простое содержимое с атрибутом max и
    ** утверждением ". le @max" (значение элемента и атрибута — xs:decimal).
    **
    ** Inheritance of assertions: Range requires low le high; Window extends
    ** Range (adds width le high - low), Positive restricts it
    ** (adds low ge 0); Money is simple content with the max attribute and
    ** the assertion ". le @max" (the element and attribute values are xs:decimal).
    */
    const char* const RANGES_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:complexType name=\"Range\"><xs:sequence><xs:element name=\"low\" type=\"xs:decimal\"/>"
        "<xs:element name=\"high\" type=\"xs:decimal\"/></xs:sequence><xs:assert test=\"low le high\"/>"
        "</xs:complexType>"
        "<xs:complexType name=\"Window\"><xs:complexContent><xs:extension base=\"Range\"><xs:sequence>"
        "<xs:element name=\"width\" type=\"xs:decimal\"/></xs:sequence><xs:assert test=\"width le high - low\"/>"
        "</xs:extension></xs:complexContent></xs:complexType>"
        "<xs:complexType name=\"Positive\"><xs:complexContent><xs:restriction base=\"Range\"><xs:sequence>"
        "<xs:element name=\"low\" type=\"xs:decimal\"/><xs:element name=\"high\" type=\"xs:decimal\"/>"
        "</xs:sequence><xs:assert test=\"low ge 0\"/></xs:restriction></xs:complexContent></xs:complexType>"
        "<xs:complexType name=\"Money\"><xs:simpleContent><xs:extension base=\"xs:decimal\">"
        "<xs:attribute name=\"max\" type=\"xs:decimal\"/><xs:assert test=\". le @max\"/></xs:extension>"
        "</xs:simpleContent></xs:complexType>"
        "<xs:element name=\"ranges\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"range\" type=\"Range\" maxOccurs=\"unbounded\"/>"
        "<xs:element name=\"money\" type=\"Money\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "</xs:sequence></xs:complexType></xs:element></xs:schema>";

    /*
    ** Заказ: у строки — total = qty * price, у заказа — сумма строк и
    ** «все количества положительны». Пространство имён urn:o с префиксом o
    ** объявлено на корне документа, а мини-дерево строки начинается ниже:
    ** префикс должен перейти на его корень.
    **
    ** An order: a line has total = qty * price, the order has the sum of the lines and
    ** "all the quantities are positive". The namespace urn:o with the prefix o
    ** is declared at the document root, and the mini-tree of a line starts below:
    ** the prefix must move onto its root.
    */
    const char* const ORDER_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xmlns:o=\"urn:o\" targetNamespace=\"urn:o\""
        " elementFormDefault=\"qualified\" xpathDefaultNamespace=\"##targetNamespace\">"
        "<xs:element name=\"order\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"line\" maxOccurs=\"unbounded\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"qty\" type=\"xs:integer\"/><xs:element name=\"price\" type=\"xs:decimal\"/>"
        "<xs:element name=\"total\" type=\"xs:decimal\"/></xs:sequence>"
        "<xs:assert test=\"o:total = o:qty * o:price\" xpathDefaultNamespace=\"##local\"/></xs:complexType></xs:element>"
        "<xs:element name=\"sum\" type=\"xs:decimal\"/></xs:sequence>"
        "<xs:assert test=\"sum = sum(line/total)\"/>"
        "<xs:assert test=\"every $l in line satisfies $l/qty gt 0\"/>"
        "</xs:complexType></xs:element></xs:schema>";

    // Строка заказа с префиксом o
    // An order line with the prefix o
    std::string orderLine(const std::string& qty, const std::string& price, const std::string& total)
    {
        return "<o:line><o:qty>" + qty + "</o:qty><o:price>" + price + "</o:price><o:total>" + total
            + "</o:total></o:line>";
    }

    /*
    ** Утверждения базового типа проверяются и у производных (xsi:type):
    ** расширение Window и ограничение Positive добавляют свои к
    ** унаследованному low le high; у простого содержимого утверждение видит
    ** значение элемента и атрибута как xs:decimal (10.50 = 10.5).
    **
    ** The assertions of the base type are checked for derived ones too (xsi:type):
    ** the extension Window and the restriction Positive add their own to the
    ** inherited low le high; for simple content an assertion sees
    ** the element and attribute values as xs:decimal (10.50 = 10.5).
    */
    void testInheritedAssertions()
    {
        std::string open = "<ranges xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">";
        std::vector<beresta::Diagnostic> errors = support::agreedSchemaErrors(RANGES_XSD, open
            + "<range><low>1</low><high>2</high></range><range><low>5</low><high>3</high></range></ranges>");

        TEST_ASSERT(errors.size() == 1);
        TEST_ASSERT_EQ(errors[0].message, "/ranges/range[2]: не выполнено утверждение low le high");
        errors = support::agreedSchemaErrors(RANGES_XSD, open + "<range xsi:type=\"Window\"><low>4</low>"
            "<high>3</high><width>0</width></range><range xsi:type=\"Window\"><low>1</low><high>3</high>"
            "<width>2</width></range></ranges>");
        TEST_ASSERT(errors.size() == 2);
        TEST_ASSERT_EQ(errors[0].message, "/ranges/range[1]: не выполнено утверждение low le high");
        TEST_ASSERT_EQ(errors[1].message, "/ranges/range[1]: не выполнено утверждение width le high - low");
        errors = support::agreedSchemaErrors(RANGES_XSD, open + "<range xsi:type=\"Positive\"><low>-1</low>"
            "<high>2</high></range><range xsi:type=\"Positive\"><low>3</low><high>2</high></range></ranges>");
        TEST_ASSERT(errors.size() == 2);
        TEST_ASSERT_EQ(errors[0].message, "/ranges/range[1]: не выполнено утверждение low ge 0");
        TEST_ASSERT_EQ(errors[1].message, "/ranges/range[2]: не выполнено утверждение low le high");
        errors = support::agreedSchemaErrors(RANGES_XSD, "<ranges><range><low>1</low><high>1</high></range>"
            "<money max=\"10.5\">10.50</money><money max=\"10.5\">10.51</money></ranges>");
        TEST_ASSERT(errors.size() == 1);
        TEST_ASSERT_EQ(errors[0].message, "/ranges/money[2]: не выполнено утверждение . le @max");
    }

    /*
    ** Вложенные элементы с утверждениями: мини-дерево строки вставляется в
    ** мини-дерево заказа, поэтому sum(line/total) видит все строки. Путь
    ** ошибки строки — /o:order/o:line[2], порядок ошибок, строки и столбцы
    ** совпадают в трёх режимах; квантор every ловит нулевое количество.
    **
    ** Nested elements with assertions: the mini-tree of a line is inserted into
    ** the mini-tree of the order, so sum(line/total) sees all the lines. The error path
    ** of a line is /o:order/o:line[2], the order of errors, the lines and columns
    ** match in the three modes; the every quantifier catches a zero quantity.
    */
    void testNestedAssertions()
    {
        std::string open = "<o:order xmlns:o=\"urn:o\">\n";
        std::vector<beresta::Diagnostic> errors = support::agreedSchemaErrors(ORDER_XSD, open
            + orderLine("3", "0.10", "0.30") + orderLine("2", "1.15", "2.30") + "<o:sum>2.60</o:sum></o:order>");

        TEST_ASSERT(errors.empty());
        errors = support::agreedSchemaErrors(ORDER_XSD, open + orderLine("3", "0.10", "0.30") + "\n"
            + orderLine("2", "1.15", "2.31") + "<o:sum>2.61</o:sum></o:order>");
        TEST_ASSERT(errors.size() == 1);
        TEST_ASSERT_EQ(errors[0].message, "/o:order/o:line[2]: не выполнено утверждение o:total = o:qty * o:price");
        TEST_ASSERT_EQ(std::to_string(errors[0].position.line) + ":" + std::to_string(errors[0].position.column), "3:1");
        errors = support::agreedSchemaErrors(ORDER_XSD, open + orderLine("0", "1", "0") + orderLine("1", "1", "1")
            + "<o:sum>2</o:sum></o:order>");
        TEST_ASSERT(errors.size() == 2);
        TEST_ASSERT_EQ(errors[0].message, "/o:order: не выполнено утверждение sum = sum(line/total)");
        TEST_ASSERT_EQ(errors[1].message, "/o:order: не выполнено утверждение every $l in line satisfies $l/qty gt 0");
    }
}

namespace suites
{
    // Регистрация тестов наследования и вложенности утверждений
    // Registers the tests of inheritance and nesting of assertions
    void registerXsdAssertRulesTests(test::TestRunner& runner)
    {
        runner.add("XSD 1.1: xs:assert, наследование", testInheritedAssertions);
        runner.add("XSD 1.1: xs:assert, вложенные элементы и пути", testNestedAssertions);
    }
}
