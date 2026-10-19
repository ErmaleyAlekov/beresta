#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

#include <map>
#include <string>

namespace
{
    const char* const XS = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">";

    // Подключаемая схема c.xsd: группа G (элемент a) и группа атрибутов A (атрибут x)
    // The included schema c.xsd: the group G (the element a) and the attribute group A (the attribute x)
    const char* const BASE_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:group name=\"G\"><xs:sequence><xs:element name=\"a\"/></xs:sequence></xs:group>"
        "<xs:attributeGroup name=\"A\"><xs:attribute name=\"x\"/></xs:attributeGroup></xs:schema>";

    /*
    ** Пишет файлы схем во временный каталог (c.xsd — всегда BASE_XSD) и
    ** загружает main.xsd через FileResourceResolver этого каталога.
    **
    ** Writes the schema files into a temporary directory (c.xsd is always BASE_XSD) and
    ** loads main.xsd through the FileResourceResolver of that directory.
    */
    std::shared_ptr<const beresta::xsd::Schema> loadFiles(const std::map<std::string, std::string>& files)
    {
        std::filesystem::path root = support::makeTestDirectory("xsd_redefine_rules");
        beresta::FileResourceResolver resolver(root);
        beresta::xsd::SchemaLoadOptions options;

        support::writeTestFile(root / "c.xsd", BASE_XSD);
        for (const auto& file : files)
            support::writeTestFile(root / file.first, file.second);
        options.resolver = &resolver;
        return beresta::xsd::Schema::fromFile(root / "main.xsd", options);
    }

    /*
    ** src-redefine.6.1 и 7.1 (msData/schema schR3, schR4, schT2): ссылка
    ** на прежнее определение — не больше одной, у группы — ровно один раз
    ** (minOccurs = maxOccurs = 1). Одна ссылка без повторений допустима.
    **
    ** src-redefine.6.1 and 7.1 (msData/schema schR3, schR4, schT2): a reference
    ** to the former definition — at most one, for a group — exactly once
    ** (minOccurs = maxOccurs = 1). One reference without repetitions is allowed.
    */
    void testSelfReferences()
    {
        std::string open = std::string(XS) + "<xs:redefine schemaLocation=\"c.xsd\">";
        std::string close = "</xs:redefine></xs:schema>";

        TEST_ASSERT_THROWS(loadFiles({{"main.xsd", open + "<xs:group name=\"G\"><xs:sequence><xs:group ref=\"G\"/>"
            "<xs:group ref=\"G\"/></xs:sequence></xs:group>" + close}}), beresta::DomException);
        TEST_ASSERT_THROWS(loadFiles({{"main.xsd", open + "<xs:group name=\"G\"><xs:choice>"
            "<xs:group ref=\"G\" minOccurs=\"0\"/><xs:element name=\"b\"/></xs:choice></xs:group>" + close}}),
            beresta::DomException);
        TEST_ASSERT_THROWS(loadFiles({{"main.xsd", open + "<xs:attributeGroup name=\"A\">"
            "<xs:attributeGroup ref=\"A\"/><xs:attributeGroup ref=\"A\"/></xs:attributeGroup>" + close}}),
            beresta::DomException);
        TEST_ASSERT(loadFiles({{"main.xsd", open + "<xs:group name=\"G\"><xs:sequence><xs:group ref=\"G\"/>"
            "<xs:element name=\"b\"/></xs:sequence></xs:group>" + close}}) != nullptr);
    }

    /*
    ** Повторное переопределение одного компонента (msData/schema schN4,
    ** schN5, schN7, schT2) — ошибка: два соседних документа переопределяют
    ** G из c.xsd, или G дважды в одном xs:redefine. Допустимы цепочка
    ** (main переопределяет b, который переопределил c) и вложенность
    ** (main включает h, оба переопределяют c — случаи schU3…schU5).
    **
    ** A repeated redefinition of one component (msData/schema schN4,
    ** schN5, schN7, schT2) is an error: two sibling documents redefine
    ** G from c.xsd, or G twice in one xs:redefine. A chain is allowed
    ** (main redefines b, which redefined c) and nesting too
    ** (main includes h, both redefine c — the cases schU3…schU5).
    */
    void testRepeatedRedefinition()
    {
        std::string body = "<xs:redefine schemaLocation=\"c.xsd\"><xs:group name=\"G\"><xs:sequence>"
            "<xs:group ref=\"G\"/><xs:element name=\"b\"/></xs:sequence></xs:group></xs:redefine>";
        std::string redefineC = std::string(XS) + body;

        TEST_ASSERT_THROWS(loadFiles({{"a.xsd", redefineC + "</xs:schema>"}, {"b.xsd", redefineC + "</xs:schema>"},
            {"main.xsd", std::string(XS) + "<xs:include schemaLocation=\"a.xsd\"/>"
            "<xs:include schemaLocation=\"b.xsd\"/></xs:schema>"}}), beresta::DomException);
        TEST_ASSERT_THROWS(loadFiles({{"main.xsd", std::string(XS) + "<xs:redefine schemaLocation=\"c.xsd\">"
            "<xs:group name=\"G\"><xs:sequence><xs:group ref=\"G\"/></xs:sequence></xs:group>"
            "<xs:group name=\"G\"><xs:sequence><xs:group ref=\"G\"/></xs:sequence></xs:group>"
            "</xs:redefine></xs:schema>"}}), beresta::DomException);
        TEST_ASSERT(loadFiles({{"h.xsd", redefineC + "</xs:schema>"}, {"main.xsd", std::string(XS)
            + "<xs:include schemaLocation=\"h.xsd\"/>" + body + "</xs:schema>"}}) != nullptr);
        auto chain = loadFiles({{"b.xsd", redefineC + "</xs:schema>"}, {"main.xsd", std::string(XS)
            + "<xs:redefine schemaLocation=\"b.xsd\"><xs:group name=\"G\"><xs:sequence><xs:group ref=\"G\"/>"
            "<xs:element name=\"z\"/></xs:sequence></xs:group></xs:redefine>"
            "<xs:element name=\"doc\"><xs:complexType><xs:group ref=\"G\"/></xs:complexType></xs:element>"
            "</xs:schema>"}});
        beresta::xsd::SchemaValidator validator(chain);
        TEST_ASSERT(validator.isValid(beresta::XmlParser().parse("<doc><a/><b/><z/></doc>")));
        TEST_ASSERT(!validator.isValid(beresta::XmlParser().parse("<doc><a/><z/></doc>")));
    }

    /*
    ** Корневой документ запоминается под своим URI: include, ведущий
    ** обратно к нему через другой файл, не загружает его второй раз
    ** (раньше — «повторное определение» всех его компонентов).
    **
    ** The root document is remembered under its URI: an include leading
    ** back to it through another file does not load it a second time
    ** (previously — a "duplicate definition" of all its components).
    */
    void testIncludeBackToRoot()
    {
        auto schema = loadFiles({{"other.xsd", std::string(XS) + "<xs:include schemaLocation=\"main.xsd\"/>"
            "<xs:element name=\"o\" type=\"T\"/></xs:schema>"}, {"main.xsd", std::string(XS)
            + "<xs:include schemaLocation=\"other.xsd\"/><xs:simpleType name=\"T\"><xs:restriction base=\"xs:int\"/>"
            "</xs:simpleType></xs:schema>"}});
        beresta::xsd::SchemaValidator validator(schema);

        TEST_ASSERT(validator.isValid(beresta::XmlParser().parse("<o>12</o>")));
        TEST_ASSERT(!validator.isValid(beresta::XmlParser().parse("<o>x</o>")));
    }
}

namespace suites
{
    // Регистрация тестов правил xs:redefine (src-redefine) и загрузки документов схемы
    // Registers the tests of the xs:redefine rules (src-redefine) and of loading schema documents
    void registerXsdRedefineRulesTests(test::TestRunner& runner)
    {
        runner.add("XSD: xs:redefine — ссылка на прежнее определение", testSelfReferences);
        runner.add("XSD: xs:redefine — повторное переопределение", testRepeatedRedefinition);
        runner.add("XSD: include обратно к корневому документу", testIncludeBackToRoot);
    }
}
