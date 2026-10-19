#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Подключаемая схема без targetNamespace («хамелеон», как в случаях
    ** W3C msData/redefine): простой тип Code, составной Person, группа
    ** Extra, группа атрибутов Common и элемент person, объявленный здесь же.
    **
    ** An included schema without targetNamespace (a "chameleon", as in the
    ** W3C msData/redefine cases): the simple type Code, the complex type Person, the group
    ** Extra, the attribute group Common and the element person declared right here.
    */
    const char* const BASE_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:simpleType name=\"Code\"><xs:restriction base=\"xs:string\"><xs:maxLength value=\"5\"/>"
        "</xs:restriction></xs:simpleType>"
        "<xs:complexType name=\"Person\"><xs:sequence><xs:element name=\"name\" type=\"xs:string\"/>"
        "<xs:group ref=\"Extra\"/></xs:sequence><xs:attributeGroup ref=\"Common\"/></xs:complexType>"
        "<xs:group name=\"Extra\"><xs:sequence><xs:element name=\"a\" type=\"xs:string\"/></xs:sequence></xs:group>"
        "<xs:attributeGroup name=\"Common\"><xs:attribute name=\"code\" type=\"Code\"/></xs:attributeGroup>"
        "<xs:element name=\"person\" type=\"Person\"/></xs:schema>";

    // Схема main.xsd с директивой directive и её содержимым, загруженная из временного каталога
    // The schema main.xsd with the directive directive and its content, loaded from a temporary directory
    std::shared_ptr<const beresta::xsd::Schema> loadWith(const std::string& directive, const std::string& body)
    {
        std::filesystem::path root = support::makeTestDirectory("xsd_redefine");
        support::writeTestFile(root / "base.xsd", BASE_XSD);
        support::writeTestFile(root / "main.xsd", "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
            " xmlns=\"urn:m\" targetNamespace=\"urn:m\"><xs:" + directive + " schemaLocation=\"base.xsd\">"
            + body + "</xs:" + directive + "></xs:schema>");
        beresta::FileResourceResolver resolver(root);
        beresta::xsd::SchemaLoadOptions options;
        options.resolver = &resolver;
        return beresta::xsd::Schema::fromFile(root / "main.xsd", options);
    }

    /*
    ** xs:redefine: каждый компонент наследует прежний под тем же именем.
    ** Code сужается до трёх символов, Person получает возраст, Extra —
    ** второй элемент b после прежнего a, Common — атрибут level. Подмена
    ** сквозная: элемент person из подключённой схемы видит новые версии.
    **
    ** xs:redefine: every component derives from the former one under the same name.
    ** Code narrows to three characters, Person gets an age, Extra — a
    ** second element b after the former a, Common — the attribute level. The replacement
    ** is pervasive: the element person from the included schema sees the new versions.
    */
    void testRedefine()
    {
        auto schema = loadWith("redefine",
            "<xs:simpleType name=\"Code\"><xs:restriction base=\"Code\"><xs:maxLength value=\"3\"/>"
            "</xs:restriction></xs:simpleType>"
            "<xs:complexType name=\"Person\"><xs:complexContent><xs:extension base=\"Person\"><xs:sequence>"
            "<xs:element name=\"age\" type=\"xs:int\"/></xs:sequence></xs:extension></xs:complexContent>"
            "</xs:complexType>"
            "<xs:group name=\"Extra\"><xs:sequence><xs:group ref=\"Extra\"/><xs:element name=\"b\" type=\"xs:int\"/>"
            "</xs:sequence></xs:group>"
            "<xs:attributeGroup name=\"Common\"><xs:attributeGroup ref=\"Common\"/>"
            "<xs:attribute name=\"level\" type=\"xs:int\"/></xs:attributeGroup>");
        beresta::xsd::SchemaValidator validator(schema);
        std::string open = "<m:person xmlns:m=\"urn:m\" code=\"abc\" level=\"2\">";

        TEST_ASSERT(validator.isValid(beresta::XmlParser().parse(open + "<name>N</name><a>x</a><b>1</b>"
            "<age>30</age></m:person>")));
        TEST_ASSERT(!validator.isValid(beresta::XmlParser().parse(open + "<name>N</name><a>x</a>"
            "<age>30</age></m:person>")));
        TEST_ASSERT(validator.validate(beresta::XmlParser().parse("<m:person xmlns:m=\"urn:m\" code=\"abcd\">"
            "<name>N</name><a>x</a><b>1</b><age>30</age></m:person>")).size() == 1);
    }

    /*
    ** xs:override (XSD 1.1): компонент заменяется целиком, без ссылки на
    ** прежний; одноимённое определение подключаемой схемы пропускается.
    **
    ** xs:override (XSD 1.1): a component is replaced entirely, without a reference to
    ** the former one; the same-named definition of the included schema is skipped.
    */
    void testOverride()
    {
        auto schema = loadWith("override", "<xs:simpleType name=\"Code\"><xs:restriction base=\"xs:int\"/>"
            "</xs:simpleType><xs:complexType name=\"Person\"><xs:sequence><xs:element name=\"id\" type=\"Code\"/>"
            "</xs:sequence></xs:complexType>");
        beresta::xsd::SchemaValidator validator(schema);

        TEST_ASSERT(validator.isValid(beresta::XmlParser().parse(
            "<m:person xmlns:m=\"urn:m\"><id>123456</id></m:person>")));
        TEST_ASSERT(!validator.isValid(beresta::XmlParser().parse(
            "<m:person xmlns:m=\"urn:m\"><id>abc</id></m:person>")));
        TEST_ASSERT(!validator.isValid(beresta::XmlParser().parse(
            "<m:person xmlns:m=\"urn:m\"><name>N</name><a>x</a></m:person>")));
    }

    /*
    ** Ошибки схемы: компонента нет в подключаемой схеме, тип не наследует
    ** самого себя, в xs:redefine элемент (допустимы лишь четыре вида),
    ** без загрузчика ресурсов.
    **
    ** Schema errors: the component is missing from the included schema, a type does not
    ** derive from itself, an element inside xs:redefine (only four kinds are allowed),
    ** no resource loader.
    */
    void testRedefineErrors()
    {
        TEST_ASSERT_THROWS(loadWith("redefine", "<xs:simpleType name=\"Missing\"><xs:restriction base=\"Missing\"/>"
            "</xs:simpleType>"), beresta::DomException);
        TEST_ASSERT_THROWS(loadWith("redefine", "<xs:simpleType name=\"Code\"><xs:restriction base=\"xs:int\"/>"
            "</xs:simpleType>"), beresta::DomException);
        TEST_ASSERT_THROWS(loadWith("redefine", "<xs:element name=\"person\" type=\"xs:string\"/>"),
            beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString("<xs:schema xmlns:xs=\""
            "http://www.w3.org/2001/XMLSchema\"><xs:redefine schemaLocation=\"base.xsd\"/></xs:schema>"),
            beresta::DomException);
    }
}

namespace suites
{
    // Регистрация тестов xs:redefine и xs:override
    // Registers the xs:redefine and xs:override tests
    void registerXsdRedefineTests(test::TestRunner& runner)
    {
        runner.add("XSD: xs:redefine", testRedefine);
        runner.add("XSD: xs:override", testOverride);
        runner.add("XSD: ошибки xs:redefine", testRedefineErrors);
    }
}
