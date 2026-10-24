#include "suites/test_suites.hpp"
#include "support/parse_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Форма xs:attribute: use у глобального объявления, default вместе с
    ** use="required", недопустимое значение use, name вместе с ref, type
    ** вместе с вложенным xs:simpleType, имя xmlns — ошибки. fixed вместе с
    ** use="required" / "prohibited" и default с use="optional" допустимы.
    **
    ** The form of xs:attribute: use on a global declaration, default together with
    ** use="required", an invalid use value, name together with ref, type
    ** together with a nested xs:simpleType, the name xmlns are errors. fixed together with
    ** use="required" / "prohibited" and default with use="optional" are allowed.
    */
    void testAttributeForm()
    {
        const char* const invalid[] = {
            "<xs:attribute name=\"a\" use=\"required\"/>",
            "<xs:complexType name=\"t\"><xs:attribute name=\"a\" default=\"1\" use=\"required\"/></xs:complexType>",
            "<xs:complexType name=\"t\"><xs:attribute name=\"a\" use=\"default\"/></xs:complexType>",
            "<xs:attribute name=\"g\"/><xs:complexType name=\"t\"><xs:attribute name=\"a\" ref=\"g\"/></xs:complexType>",
            "<xs:complexType name=\"t\"><xs:attribute name=\"a\" type=\"xs:int\"><xs:simpleType><xs:restriction base=\"xs:int\"/></xs:simpleType></xs:attribute></xs:complexType>",
            "<xs:complexType name=\"t\"><xs:attribute name=\"xmlns\"/></xs:complexType>"
        };

        for (const char* body : invalid)
            TEST_ASSERT_THROWS(support::schemaFromBody(body), beresta::DomException);

        TEST_ASSERT(support::schemaFromBody("<xs:complexType name=\"t\">"
            "<xs:attribute name=\"a\" fixed=\"1\" use=\"required\"/>"
            "<xs:attribute name=\"b\" fixed=\"1\" use=\"prohibited\"/>"
            "<xs:attribute name=\"c\" default=\"1\" use=\"optional\"/>"
            "</xs:complexType>") != nullptr);
    }

    /*
    ** Вложенность xs:all: sequence внутри all, all внутри sequence и ссылка на
    ** группу с all внутри choice — ошибки; группа с all как вся модель типа допустима.
    **
    ** The nesting of xs:all: sequence inside all, all inside sequence and a reference to
    ** a group with all inside choice are errors; a group with all as the whole model of a type is allowed.
    */
    void testAllNesting()
    {
        const char* const invalid[] = {
            "<xs:complexType name=\"t\"><xs:all><xs:sequence><xs:element name=\"a\"/></xs:sequence></xs:all></xs:complexType>",
            "<xs:complexType name=\"t\"><xs:sequence><xs:all><xs:element name=\"a\"/></xs:all></xs:sequence></xs:complexType>",
            "<xs:group name=\"g\"><xs:all><xs:element name=\"a\"/></xs:all></xs:group><xs:complexType name=\"t\"><xs:choice><xs:group ref=\"g\"/></xs:choice></xs:complexType>"
        };

        for (const char* body : invalid)
            TEST_ASSERT_THROWS(support::schemaFromBody(body), beresta::DomException);

        TEST_ASSERT(support::schemaFromBody("<xs:group name=\"g\"><xs:all><xs:element name=\"a\"/></xs:all></xs:group>"
            "<xs:complexType name=\"t\"><xs:group ref=\"g\"/></xs:complexType>") != nullptr);
    }

    /*
    ** Фасеты одного шага: totalDigits="0", fractionDigits больше totalDigits,
    ** fractionDigits у xs:int, граница вне базового типа или пустая, две верхние
    ** границы, min больше max, length с minLength, minLength больше maxLength,
    ** граница у строки, enumeration у boolean, enumeration не по базовому типу,
    ** ослабленный whiteSpace, повтор фасета — ошибки. Согласованные фасеты допустимы.
    **
    ** The facets of one step: totalDigits="0", fractionDigits greater than totalDigits,
    ** fractionDigits on xs:int, a bound outside the base type or empty, two upper
    ** bounds, min greater than max, length with minLength, minLength greater than maxLength,
    ** a bound on a string, enumeration on boolean, an enumeration not of the base type,
    ** a weakened whiteSpace, a repeated facet are errors. Consistent facets are allowed.
    */
    void testFacetValues()
    {
        const char* const invalid[] = {
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:decimal\"><xs:totalDigits value=\"0\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:decimal\"><xs:totalDigits value=\"3\"/><xs:fractionDigits value=\"4\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:int\"><xs:fractionDigits value=\"1\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:unsignedShort\"><xs:minInclusive value=\"-1\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:int\"><xs:minInclusive value=\"\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:date\"><xs:maxInclusive value=\"2000-01-01\"/><xs:maxExclusive value=\"2000-01-01\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:int\"><xs:minInclusive value=\"7\"/><xs:maxInclusive value=\"1\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:string\"><xs:length value=\"5\"/><xs:minLength value=\"1\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:string\"><xs:minLength value=\"5\"/><xs:maxLength value=\"2\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:string\"><xs:maxInclusive value=\"a\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:boolean\"><xs:enumeration value=\"true\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:decimal\"><xs:enumeration value=\"abc\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:token\"><xs:whiteSpace value=\"preserve\"/></xs:restriction></xs:simpleType>",
            "<xs:simpleType name=\"s\"><xs:restriction base=\"xs:int\"><xs:minInclusive value=\"1\"/><xs:minInclusive value=\"2\"/></xs:restriction></xs:simpleType>"
        };

        for (const char* body : invalid)
            TEST_ASSERT_THROWS(support::schemaFromBody(body), beresta::DomException);

        TEST_ASSERT(support::schemaFromBody("<xs:simpleType name=\"s\"><xs:restriction base=\"xs:int\">"
            "<xs:minInclusive value=\"1\"/><xs:maxExclusive value=\"10\"/></xs:restriction></xs:simpleType>") != nullptr);
        TEST_ASSERT(support::schemaFromBody("<xs:simpleType name=\"s\"><xs:restriction base=\"xs:decimal\">"
            "<xs:totalDigits value=\"5\"/><xs:fractionDigits value=\"2\"/></xs:restriction></xs:simpleType>") != nullptr);
        TEST_ASSERT(support::schemaFromBody("<xs:simpleType name=\"s\"><xs:restriction base=\"xs:string\">"
            "<xs:minLength value=\"1\"/><xs:maxLength value=\"3\"/></xs:restriction></xs:simpleType>") != nullptr);
        TEST_ASSERT(support::schemaFromBody("<xs:simpleType name=\"s\"><xs:restriction base=\"xs:string\">"
            "<xs:whiteSpace value=\"collapse\"/><xs:enumeration value=\" a \"/></xs:restriction></xs:simpleType>") != nullptr);
    }

    /*
    ** Граница производного типа не выходит за границу базового: maxInclusive="11"
    ** под maxInclusive="10" — ошибка, равная граница допустима.
    **
    ** The bound of a derived type does not go beyond the bound of the base one: maxInclusive="11"
    ** under maxInclusive="10" is an error, an equal bound is allowed.
    */
    void testFacetBaseBounds()
    {
        const char* const invalid[] = {
            "<xs:simpleType name=\"b\"><xs:restriction base=\"xs:int\"><xs:maxInclusive value=\"10\"/></xs:restriction></xs:simpleType>"
            "<xs:simpleType name=\"s\"><xs:restriction base=\"b\"><xs:maxInclusive value=\"11\"/></xs:restriction></xs:simpleType>"
        };

        for (const char* body : invalid)
            TEST_ASSERT_THROWS(support::schemaFromBody(body), beresta::DomException);

        TEST_ASSERT(support::schemaFromBody("<xs:simpleType name=\"b\"><xs:restriction base=\"xs:int\"><xs:maxInclusive value=\"10\"/></xs:restriction></xs:simpleType>"
            "<xs:simpleType name=\"s\"><xs:restriction base=\"b\"><xs:maxInclusive value=\"10\"/></xs:restriction></xs:simpleType>") != nullptr);
    }
}

namespace suites
{
    // Регистрация тестов правил схемы: форма xs:attribute, вложенность xs:all, фасеты
    // Registers the tests of schema rules: the form of xs:attribute, the nesting of xs:all, facets
    void registerXsdSchemaRulesTests(test::TestRunner& runner)
    {
        runner.add("XSD: правила использования атрибутов xs:attribute", testAttributeForm);
        runner.add("XSD: правила вложения xs:all, xs:sequence и xs:choice", testAllNesting);
        runner.add("XSD: правила использования фасетов xs:simpleType", testFacetValues);
        runner.add("XSD: правила наследования фасетов", testFacetBaseBounds);
    }
}
