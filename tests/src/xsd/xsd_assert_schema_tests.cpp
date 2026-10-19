#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Текст ошибки загрузки схемы с одним xs:assert test (пустая строка —
    ** схема загрузилась). Утверждение стоит на третьей строке документа схемы.
    **
    ** The text of the loading error of a schema with one xs:assert test (an empty string —
    ** the schema loaded). The assertion is on the third line of the schema document.
    */
    std::string loadError(const std::string& test)
    {
        std::string xsd = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">\n"
            "<xs:element name=\"t\"><xs:complexType><xs:sequence><xs:element name=\"item\" type=\"xs:string\""
            " maxOccurs=\"unbounded\"/></xs:sequence><xs:attribute name=\"a\" type=\"xs:string\"/>\n"
            "<xs:assert test=\"" + test + "\"/></xs:complexType></xs:element></xs:schema>";

        try
        {
            beresta::xsd::Schema::fromString(xsd);
        }
        catch (const beresta::DomException& error)
        {
            return error.getMessage();
        }
        return std::string();
    }

    /*
    ** Неподдерживаемые конструкции XPath 2.0 — ошибка ЗАГРУЗКИ схемы с
    ** номером строки xs:assert и текстом утверждения, а не «тихое» true
    ** при проверке: for, instance of, cast / castable, to, idiv, except,
    ** is, последовательности, экспонента, функции и конструкторы вне набора,
    ** неполные if / some, абсолютные пути (корень дерева утверждения — сам
    ** элемент). Поддерживаемые выражения загружаются.
    **
    ** Unsupported XPath 2.0 constructs are a schema LOADING error with
    ** the line number of xs:assert and the assertion text, not a "silent" true
    ** during validation: for, instance of, cast / castable, to, idiv, except,
    ** is, sequences, an exponent, functions and constructors outside the set,
    ** incomplete if / some, absolute paths (the root of the assertion tree is the
    ** element itself). Supported expressions load.
    */
    void testUnsupportedXPath2()
    {
        const char* const UNSUPPORTED[] = {
            "for $x in item return $x", "@a instance of xs:string", "@a cast as xs:integer",
            "@a castable as xs:integer", "count(1 to 5) = 5", "@a idiv 2 = 1", "item except item[1]",
            "item[1] is item[2]", "(1, 2) = 1", "1e3 = 1000", "fn:matches(@a, 'x')", "matches(@a, 'x')",
            "string-join(item, ',') = 'x'", "xs:time('10:00:00') = xs:time('10:00:00')", "xs:decimal(1, 2) = 1",
            "if (@a) then true()", "some $x in item", "every x in item satisfies true()", "@a = 'it''s'",
            "unknown:fn(@a)", "(: comment :) true()", "exists()", "/t/item", "count(//item) = 2"
        };

        for (const char* test : UNSUPPORTED)
        {
            std::string message = loadError(test);
            TEST_ASSERT(message.rfind("схема, строка 3: утверждение test=\"" + std::string(test) + "\"", 0) == 0);
        }
        TEST_ASSERT(loadError("count(item) ge 1 and (if (@a) then @a ne '' else true())").empty());
        TEST_ASSERT(loadError("every $i in item satisfies some $j in item satisfies $i = $j").empty());
        TEST_ASSERT(loadError("xs:date('2020-01-01') lt xs:date('2021-01-01') and exists(item)").empty());
        TEST_ASSERT(loadError("").rfind("схема, строка 3: утверждение test=\"\"", 0) == 0);
    }

    /*
    ** Схема фасетов xs:assertion: Even, SmallEven, Price, Recent и объединение
    ** Mixed; элемент v с детьми этих типов и атрибутом even.
    **
    ** The schema of xs:assertion facets: Even, SmallEven, Price, Recent and the union
    ** Mixed; the element v with children of these types and the attribute even.
    */
    const char* const FACETS_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:simpleType name=\"Even\"><xs:restriction base=\"xs:integer\"><xs:assertion test=\"$value mod 2 = 0\"/>"
        "</xs:restriction></xs:simpleType><xs:simpleType name=\"SmallEven\"><xs:restriction base=\"Even\">"
        "<xs:assertion test=\"$value lt 50\"/></xs:restriction></xs:simpleType>"
        "<xs:simpleType name=\"Price\"><xs:restriction base=\"xs:decimal\">"
        "<xs:assertion test=\"$value * 100 = floor($value * 100)\"/></xs:restriction></xs:simpleType>"
        "<xs:simpleType name=\"Recent\"><xs:restriction base=\"xs:date\">"
        "<xs:assertion test=\"$value ge xs:date('2000-01-01')\"/></xs:restriction></xs:simpleType>"
        "<xs:simpleType name=\"Mixed\"><xs:restriction><xs:simpleType><xs:union memberTypes=\"xs:integer xs:date\"/>"
        "</xs:simpleType><xs:assertion test=\"string($value) != '0'\"/></xs:restriction></xs:simpleType>"
        "<xs:element name=\"v\"><xs:complexType><xs:sequence><xs:element name=\"small\" type=\"SmallEven\"/>"
        "<xs:element name=\"price\" type=\"Price\"/><xs:element name=\"recent\" type=\"Recent\"/>"
        "<xs:element name=\"mixed\" type=\"Mixed\" maxOccurs=\"2\"/></xs:sequence>"
        "<xs:attribute name=\"even\" type=\"Even\"/></xs:complexType></xs:element></xs:schema>";

    /*
    ** Фасет xs:assertion: $value — типизированное значение. Even (чётные
    ** xs:integer) и его ограничение SmallEven (утверждения складываются),
    ** Price — не больше двух знаков дробной части, проверенных точной
    ** арифметикой, Recent — дата не раньше 2000 года, Mixed — объединение
    ** integer | date: $value типизирован подошедшим членом.
    **
    ** The xs:assertion facet: $value is the typed value. Even (even
    ** xs:integer) and its restriction SmallEven (the assertions add up),
    ** Price — at most two fraction digits checked with exact
    ** arithmetic, Recent — a date not earlier than 2000, Mixed — the union
    ** integer | date: $value is typed by the matching member.
    */
    void testAssertionFacets()
    {
        std::vector<beresta::Diagnostic> errors = support::agreedSchemaErrors(FACETS_XSD, "<v even=\"4\"><small>48</small>"
            "<price>1.25</price><recent>2001-02-03</recent><mixed>5</mixed><mixed>2020-01-01</mixed></v>");

        TEST_ASSERT_EQ(support::describeDiagnostics(errors), "");
        errors = support::agreedSchemaErrors(FACETS_XSD, "<v even=\"3\"><small>7</small><price>1.255</price>"
            "<recent>1999-12-31</recent><mixed>0</mixed></v>");
        TEST_ASSERT(errors.size() == 5);
        TEST_ASSERT(errors[0].message.find("\"3\" не удовлетворяет утверждению $value mod 2 = 0") != std::string::npos);
        TEST_ASSERT_EQ(errors[1].message, "/v/small: значение \"7\" не удовлетворяет утверждению $value mod 2 = 0");
        TEST_ASSERT_EQ(errors[2].message, "/v/price: значение \"1.255\" не удовлетворяет утверждению "
            "$value * 100 = floor($value * 100)");
        TEST_ASSERT(errors[3].message.rfind("/v/recent: значение \"1999-12-31\"", 0) == 0);
        TEST_ASSERT(errors[4].message.rfind("/v/mixed: значение \"0\"", 0) == 0);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString("<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
            "<xs:simpleType name=\"L\"><xs:restriction><xs:simpleType><xs:list itemType=\"xs:int\"/></xs:simpleType>"
            "<xs:assertion test=\"true()\"/></xs:restriction></xs:simpleType></xs:schema>"), beresta::DomException);
    }

    /*
    ** vc:minVersion / vc:maxVersion: «Берёста» — процессор 1.1. Компоненты
    ** с minVersion 1.2 или 2.0 и с maxVersion 1.1 пропускаются (второе
    ** объявление r не конфликтует с первым), с minVersion 1.0 и
    ** maxVersion 1.2 — остаются; xs:schema с minVersion 2.0 пуста целиком.
    **
    ** vc:minVersion / vc:maxVersion: Beresta is a 1.1 processor. Components
    ** with minVersion 1.2 or 2.0 and with maxVersion 1.1 are skipped (the second
    ** declaration of r does not conflict with the first one), with minVersion 1.0 and
    ** maxVersion 1.2 — stay; xs:schema with minVersion 2.0 is empty entirely.
    */
    void testVersioning()
    {
        std::string open = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
            " xmlns:vc=\"http://www.w3.org/2007/XMLSchema-versioning\"";
        std::string xsd = open + "><xs:element name=\"r\"><xs:complexType><xs:sequence>"
            "<xs:element name=\"new\" type=\"xs:int\" vc:minVersion=\"1.2\"/><xs:element name=\"a\" type=\"xs:int\"/>"
            "<xs:element name=\"old\" type=\"xs:int\" vc:maxVersion=\"1.1\"/>"
            "<xs:element name=\"b\" type=\"xs:int\" vc:minVersion=\"1.0\" vc:maxVersion=\"1.2\"/></xs:sequence>"
            "<xs:assert test=\"a lt b\" vc:minVersion=\"1.1\"/><xs:assert test=\"false()\" vc:minVersion=\"2.0\"/>"
            "</xs:complexType></xs:element><xs:element name=\"r\" type=\"xs:string\" vc:minVersion=\"3.0\"/>"
            "</xs:schema>";

        TEST_ASSERT(support::agreedSchemaErrors(xsd, "<r><a>1</a><b>2</b></r>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(xsd, "<r><new>1</new><a>1</a><b>2</b></r>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(xsd, "<r><a>1</a><old>1</old><b>2</b></r>").size() == 1);
        std::vector<beresta::Diagnostic> errors = support::agreedSchemaErrors(xsd, "<r><a>3</a><b>2</b></r>");
        TEST_ASSERT(errors.size() == 1);
        TEST_ASSERT_EQ(errors[0].message, "/r: не выполнено утверждение a lt b");
        errors = support::agreedSchemaErrors(open + " vc:minVersion=\"2.0\"><xs:element name=\"r\"/></xs:schema>",
            "<r/>");
        TEST_ASSERT(errors.size() == 1);
    }
}

namespace suites
{
    // Регистрация тестов загрузки утверждений, фасета xs:assertion и vc:minVersion
    // Registers the tests of assertion loading, the xs:assertion facet and vc:minVersion
    void registerXsdAssertSchemaTests(test::TestRunner& runner)
    {
        runner.add("XSD 1.1: неподдерживаемый XPath 2.0 — ошибка загрузки", testUnsupportedXPath2);
        runner.add("XSD 1.1: фасет xs:assertion", testAssertionFacets);
        runner.add("XSD 1.1: vc:minVersion и vc:maxVersion", testVersioning);
    }
}
