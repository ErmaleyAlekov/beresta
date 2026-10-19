#include "suites/test_suites.hpp"
#include "support/parse_helpers.hpp"

#include <string>

namespace
{
    /*
    ** block / final по схеме для схем (случаи W3C msData/element elemC*,
    ** elemF*, sunData/ElemDecl disallowedSubst00202m*, substGrpExcl00402m*):
    ** регистр значим, "#all" — только целиком, у элемента в final нет
    ** substitution, у составного типа в block — тоже, list/union — только
    ** у простых типов. Пустое значение — пустой список, оно допустимо.
    **
    ** block / final by the schema for schemas (the W3C cases msData/element elemC*,
    ** elemF*, sunData/ElemDecl disallowedSubst00202m*, substGrpExcl00402m*):
    ** case matters, "#all" — only alone, an element's final has no
    ** substitution, a complex type's block neither, list/union — only
    ** for simple types. An empty value is an empty list, it is allowed.
    */
    void testDerivationSetValues()
    {
        const char* const invalid[] = {
            "<xs:element name=\"e\" block=\"foo\"/>", "<xs:element name=\"e\" block=\"#All\"/>",
            "<xs:element name=\"e\" block=\"Extension\"/>", "<xs:element name=\"e\" block=\"#all extension\"/>",
            "<xs:element name=\"e\" block=\"list\"/>", "<xs:element name=\"e\" final=\"substitution\"/>",
            "<xs:complexType name=\"t\" block=\"substitution\"/>", "<xs:complexType name=\"t\" final=\"union\"/>",
            "<xs:simpleType name=\"s\" final=\"extension\"><xs:restriction base=\"xs:int\"/></xs:simpleType>"
        };

        for (const char* body : invalid)
            TEST_ASSERT_THROWS(support::schemaFromBody(body), beresta::DomException);
        TEST_ASSERT(support::schemaFromBody("<xs:element name=\"e\" block=\"substitution  extension\" final=\"\"/>"
            "<xs:complexType name=\"t\" block=\"#all\" final=\"restriction extension\"/>"
            "<xs:simpleType name=\"s\" final=\"list union\"><xs:restriction base=\"xs:int\"/></xs:simpleType>")
            != nullptr);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString("<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
            " blockDefault=\"list\"/>"), beresta::DomException);
    }

    /*
    ** Логические атрибуты abstract, nillable, mixed — значения xs:boolean
    ** (msData/element elemB*, elemK*, complexType ctA004…ctA008):
    ** "True", "", "yes" — ошибки схемы, а "1" — истина (раньше «1» молча
    ** считалось ложью, и абстрактный элемент оказывался обычным).
    **
    ** The boolean attributes abstract, nillable, mixed are xs:boolean values
    ** (msData/element elemB*, elemK*, complexType ctA004…ctA008):
    ** "True", "", "yes" are schema errors, and "1" is true (previously "1" was silently
    ** taken as false, and an abstract element became an ordinary one).
    */
    void testBooleanValues()
    {
        TEST_ASSERT_THROWS(support::schemaFromBody("<xs:element name=\"e\" abstract=\"True\"/>"), beresta::DomException);
        TEST_ASSERT_THROWS(support::schemaFromBody("<xs:element name=\"e\" nillable=\"\"/>"), beresta::DomException);
        TEST_ASSERT_THROWS(support::schemaFromBody("<xs:complexType name=\"t\" abstract=\"true false\"/>"), beresta::DomException);
        TEST_ASSERT_THROWS(support::schemaFromBody("<xs:complexType name=\"t\" mixed=\"yes\"/>"), beresta::DomException);
        auto schema = support::schemaFromBody("<xs:element name=\"e\" abstract=\" 1 \" nillable=\"0\"/>");
        TEST_ASSERT(schema->findElement("", "e")->isAbstract);
        TEST_ASSERT(!schema->findElement("", "e")->nillable);
        TEST_ASSERT(!beresta::xsd::SchemaValidator(schema).isValid(beresta::XmlParser().parse("<e/>")));
    }

    /*
    ** Форма xs:element по месту (src-element.2.2, localElement,
    ** topLevelElement; случаи errC005, schZ001, schZ002, name00501m10,
    ** addB045): у локального нет abstract / final / substitutionGroup, у
    ** ссылки ref — ничего, кроме minOccurs / maxOccurs / id и аннотации,
    ** у глобального — minOccurs, maxOccurs, form и ref.
    **
    ** The form of xs:element by its place (src-element.2.2, localElement,
    ** topLevelElement; the cases errC005, schZ001, schZ002, name00501m10,
    ** addB045): a local one has no abstract / final / substitutionGroup, a
    ** ref reference has nothing but minOccurs / maxOccurs / id and an annotation,
    ** a global one has no minOccurs, maxOccurs, form and ref.
    */
    void testElementPlaces()
    {
        std::string global = "<xs:element name=\"g\"/>";
        const char* const invalid[] = {
            "<xs:element name=\"l\" abstract=\"true\"/>", "<xs:element name=\"l\" final=\"#all\"/>",
            "<xs:element name=\"l\" substitutionGroup=\"g\"/>", "<xs:element ref=\"g\" block=\"#all\"/>",
            "<xs:element ref=\"g\" form=\"qualified\"/>", "<xs:element ref=\"g\" name=\"g\"/>",
            "<xs:element ref=\"g\"><xs:complexType/></xs:element>"
        };

        for (const char* local : invalid)
            TEST_ASSERT_THROWS(support::schemaFromBody(global + "<xs:complexType name=\"t\"><xs:sequence>" + local
                + "</xs:sequence></xs:complexType>"), beresta::DomException);
        TEST_ASSERT_THROWS(support::schemaFromBody("<xs:element name=\"g\" minOccurs=\"1\"/>"), beresta::DomException);
        TEST_ASSERT(support::schemaFromBody(global + "<xs:complexType name=\"t\"><xs:sequence>"
            "<xs:element ref=\"g\" id=\"r\" minOccurs=\"0\" maxOccurs=\"2\"><xs:annotation/></xs:element>"
            "<xs:element name=\"l\" nillable=\"true\" block=\"#all\" form=\"qualified\"/>"
            "</xs:sequence></xs:complexType>") != nullptr);
    }

    /*
    ** Именованные группы строятся при загрузке, даже если на них никто не
    ** ссылается: ошибка внутри неиспользуемой группы видна сразу. У группы
    ** и её модели нет minOccurs / maxOccurs (msData/group groupD002,
    ** modelGroups mgO027). Локальный элемент группы, чей тип ссылается на
    ** ту же группу (msData/additional addB077), — один компонент, а не
    ** бесконечная подстановка (раньше — переполнение стека).
    **
    ** Named groups are built at loading even if nobody refers to them:
    ** an error inside an unused group shows up at once. A group
    ** and its model have no minOccurs / maxOccurs (msData/group groupD002,
    ** modelGroups mgO027). A local element of a group whose type refers to
    ** the same group (msData/additional addB077) is one component, not an
    ** endless substitution (previously — a stack overflow).
    */
    void testNamedGroups()
    {
        TEST_ASSERT_THROWS(support::schemaFromBody("<xs:group name=\"g\"><xs:sequence><xs:element name=\"a\" type=\"Missing\"/>"
            "</xs:sequence></xs:group>"), beresta::DomException);
        TEST_ASSERT_THROWS(support::schemaFromBody("<xs:group name=\"g\" maxOccurs=\"1\"><xs:sequence/></xs:group>"),
            beresta::DomException);
        TEST_ASSERT_THROWS(support::schemaFromBody("<xs:group name=\"g\"><xs:all minOccurs=\"0\"/></xs:group>"),
            beresta::DomException);
        auto schema = support::schemaFromBody("<xs:complexType name=\"base\"><xs:sequence><xs:element name=\"r\"/></xs:sequence>"
            "</xs:complexType><xs:group name=\"foo\"><xs:sequence><xs:element name=\"x\" minOccurs=\"0\">"
            "<xs:complexType><xs:complexContent><xs:extension base=\"base\"><xs:group ref=\"foo\"/></xs:extension>"
            "</xs:complexContent></xs:complexType></xs:element></xs:sequence></xs:group>"
            "<xs:element name=\"doc\"><xs:complexType><xs:group ref=\"foo\"/></xs:complexType></xs:element>");
        beresta::xsd::SchemaValidator validator(schema);
        TEST_ASSERT(validator.isValid(beresta::XmlParser().parse("<doc><x><r/><x><r/></x></x></doc>")));
        TEST_ASSERT(!validator.isValid(beresta::XmlParser().parse("<doc><x><x/></x></doc>")));
    }
}

namespace suites
{
    // Регистрация тестов лексики и формы документа схемы (по случаям W3C XSTS)
    // Registers the tests of the lexical form and shape of a schema document (by the W3C XSTS cases)
    void registerXsdSchemaLexicalTests(test::TestRunner& runner)
    {
        runner.add("XSD: значения block и final", testDerivationSetValues);
        runner.add("XSD: логические атрибуты схемы", testBooleanValues);
        runner.add("XSD: атрибуты xs:element по месту объявления", testElementPlaces);
        runner.add("XSD: именованные группы при загрузке", testNamedGroups);
    }
}
