#include "suites/test_suites.hpp"
#include "support/parse_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Проверка правил размещения элементов xs:element в схеме:
    ** - ошибка при наличии вложенного xs:complexType с type="xs:int";
    ** - ошибка при наличии вложенного xs:simpleType с restriction base=xs:int;
    ** - ошибка при наличии вложенных и xs:simpleType, и xs:complexType;
    ** - допустимо — элемент только с вложенным xs:complexType.
    **
    ** Validation of xs:element placement rules in the schema:
    ** - error when nested xs:complexType with type="xs:int";
    ** - error when nested xs:simpleType with restriction base=xs:int;
    ** - error when nested both xs:simpleType and xs:complexType;
    ** - allowed — element only with nested xs:complexType.
    */
    void testElementTypeParts()
    {
        const char* const invalid[] = {
            "<xs:element name=\"e\" type=\"xs:int\"><xs:complexType/></xs:element>",
            "<xs:element name=\"e\" type=\"xs:int\"><xs:simpleType><xs:restriction base=\"xs:int\"/></xs:simpleType></xs:element>",
            "<xs:element name=\"e\" type=\"xs:int\"><xs:simpleType><xs:restriction base=\"xs:int\"/></xs:simpleType><xs:complexType/></xs:element>"
        };

        for (const char* body : invalid)
            TEST_ASSERT_THROWS(support::schemaFromBody(body), beresta::DomException);

        TEST_ASSERT(support::schemaFromBody("<xs:element name=\"e\"><xs:complexType/></xs:element>") != nullptr);
    }

    /*
    ** Проверка правил значений ограничений в элементах:
    ** - ошибка при элементе type="xs:boolean" с default="yes";
    ** - ошибка при элементе type="xs:int" с fixed="abc";
    ** - ошибка при элементе с анонимным complexType из xs:sequence с элементом и default="x";
    ** - ошибка при атрибуте внутри complexType name="t" с type="xs:int" и default="abc";
    ** - допустимо — элемент type="xs:boolean" с fixed="true";
    ** - допустимо — элемент с complexType mixed="true" (sequence с minOccurs="0" элементом) и default="любой текст";
    ** - допустимо — атрибут type="xs:int" с default=" 5 ".
    **
    ** Validation of value constraints in elements:
    ** - error when element type="xs:boolean" with default="yes";
    ** - error when element type="xs:int" with fixed="abc";
    ** - error when element with anonymous complexType from xs:sequence with element and default="x";
    ** - error when attribute inside complexType name="t" with type="xs:int" and default="abc";
    ** - allowed — element type="xs:boolean" with fixed="true";
    ** - allowed — element with complexType mixed="true" (sequence with minOccurs="0" element) and default="any text";
    ** - allowed — attribute type="xs:int" with default=" 5 ".
    */
    void testValueConstraints()
    {
        const char* const invalid[] = {
            "<xs:element name=\"e\" type=\"xs:boolean\" default=\"yes\"/>",
            "<xs:element name=\"e\" type=\"xs:int\" fixed=\"abc\"/>",
            "<xs:element name=\"e\" default=\"x\"><xs:complexType><xs:sequence><xs:element name=\"a\"/></xs:sequence></xs:complexType></xs:element>",
            "<xs:element name=\"e\"><xs:complexType><xs:attribute name=\"t\" type=\"xs:int\" default=\"abc\"/></xs:complexType></xs:element>"
        };

        for (const char* body : invalid)
            TEST_ASSERT_THROWS(support::schemaFromBody(body), beresta::DomException);

        TEST_ASSERT(support::schemaFromBody("<xs:element name=\"e\" type=\"xs:boolean\" fixed=\"true\"/>") != nullptr);
        TEST_ASSERT(support::schemaFromBody("<xs:element name=\"e\" default=\"any text\"><xs:complexType mixed=\"true\"><xs:sequence><xs:element name=\"a\" minOccurs=\"0\"/></xs:sequence></xs:complexType></xs:element>") != nullptr);
        TEST_ASSERT(support::schemaFromBody("<xs:element name=\"e\"><xs:complexType><xs:attribute name=\"t\" type=\"xs:int\" default=\" 5 \"/></xs:complexType></xs:element>") != nullptr);
    }

    /*
    ** Проверка правил размещения xs:annotation в схеме:
    ** - ошибка при наличии двух xs:annotation подряд внутри элемента;
    ** - ошибка при наличии xs:annotation после xs:selector внутри xs:key;
    ** - ошибка при наличии xs:annotation внутри xs:annotation в глобальном элементе;
    ** - ошибка при наличии двух xs:annotation внутри xs:list внутри simpleType;
    ** - допустимо — xs:key с одной xs:annotation первой, затем selector и field.
    **
    ** Validation of xs:annotation placement rules in the schema:
    ** - error when two xs:annotation are placed in a row inside an element;
    ** - error when xs:annotation is placed after xs:selector inside xs:key;
    ** - error when xs:annotation is placed inside xs:annotation in a global element;
    ** - error when two xs:annotation are placed inside xs:list inside simpleType;
    ** - allowed — xs:key with one xs:annotation first, then selector and field.
    */
    void testAnnotationPlace()
    {
        const char* const invalid[] = {
            "<xs:element name=\"e\"><xs:annotation/><xs:annotation/></xs:element>",
            "<xs:element name=\"e\"><xs:key name=\"k\"><xs:selector xpath=\".\"/><xs:annotation/><xs:field xpath=\"@a\"/></xs:key></xs:element>",
            "<xs:element name=\"e\"><xs:key name=\"k\"><xs:annotation/><xs:annotation/><xs:selector xpath=\".\"/><xs:field xpath=\"@a\"/></xs:key></xs:element>",
            "<xs:element name=\"e\"><xs:annotation><xs:annotation/></xs:annotation></xs:element>",
            "<xs:simpleType name=\"s\"><xs:list><xs:annotation/><xs:annotation/></xs:list></xs:simpleType>"
        };

        for (const char* body : invalid)
            TEST_ASSERT_THROWS(support::schemaFromBody(body), beresta::DomException);

        TEST_ASSERT(support::schemaFromBody("<xs:element name=\"e\"><xs:key name=\"k\"><xs:annotation/>"
            "<xs:selector xpath=\".\"/><xs:field xpath=\"@a\"/></xs:key></xs:element>") != nullptr);
    }

    /*
    ** Проверка правил размещения xs:notation в схеме:
    ** - ошибка при отсутствии public и system в xs:notation;
    ** - ошибка при наличии двух xs:notation с одинаковым name;
    ** - ошибка при наличии xs:notation с именем, содержащим двоеточие;
    ** - ошибка при наличии xs:notation без name и с public;
    ** - ошибка при наличии xs:notation внутри xs:annotation глобального элемента;
    ** - ошибка при наличии xs:element внутри xs:notation;
    ** - допустимо — xs:notation с name и system вместе с xs:notation с name и public.
    **
    ** Validation of xs:notation placement rules in the schema:
    ** - error when xs:notation lacks both public and system;
    ** - error when two xs:notation have the same name;
    ** - error when xs:notation has a name containing a colon;
    ** - error when xs:notation lacks name and has public;
    ** - error when xs:notation is inside xs:annotation of a global element;
    ** - error when xs:element is inside xs:notation;
    ** - allowed — xs:notation with name and system together with xs:notation with name and public.
    */
    void testNotations()
    {
        const char* const invalid[] = {
            "<xs:notation name=\"n\"/>",
            "<xs:notation name=\"n\" public=\"p\"/><xs:notation name=\"n\" public=\"p\"/>",
            "<xs:notation name=\"a:b\" public=\"p\"/>",
            "<xs:notation public=\"p\"/>",
            "<xs:element name=\"e\"><xs:annotation><xs:notation name=\"n\" public=\"p\"/></xs:annotation></xs:element>",
            "<xs:notation name=\"n\" public=\"p\"><xs:element name=\"x\"/></xs:notation>"
        };

        for (const char* body : invalid)
            TEST_ASSERT_THROWS(support::schemaFromBody(body), beresta::DomException);

        TEST_ASSERT(support::schemaFromBody("<xs:notation name=\"n\" system=\"s\"/><xs:notation name=\"m\" public=\"p\"/>") != nullptr);
    }
}

namespace suites
{
    // Регистрация тестов правил размещения элементов схемы
    // Registers the tests of schema element placement rules
    void registerXsdSchemaPlacementTests(test::TestRunner& runner)
    {
        runner.add("XSD: правила размещения элементов xs:element", testElementTypeParts);
        runner.add("XSD: правила значений ограничений", testValueConstraints);
        runner.add("XSD: правила размещения xs:annotation", testAnnotationPlace);
        runner.add("XSD: правила размещения xs:notation", testNotations);
    }
}
