#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Внешний DTD с параметрической сущностью и условной секцией
    // An external DTD with a parameter entity and a conditional section
    void testExternalDtd()
    {
        std::filesystem::path root = support::makeTestDirectory("external_dtd");
        support::writeTestFile(root / "doc.dtd",
            "<!ENTITY % content \"(#PCDATA)\"><![INCLUDE[<!ELEMENT doc %content;>]]>"
            "<![IGNORE[<!ELEMENT broken>]]><!ATTLIST doc v CDATA \"по умолчанию\">");
        support::writeTestFile(root / "doc.xml", "<!DOCTYPE doc SYSTEM \"doc.dtd\"><doc>x</doc>");
        beresta::FileResourceResolver resolver(root);
        beresta::ParserOptions options;
        options.resolver = &resolver;
        options.loadExternalDtd = true;
        options.validateDtd = true;

        beresta::Document document = beresta::XmlParser(options).parseFile(root / "doc.xml");
        TEST_ASSERT_EQ(document.getRoot()->getAttribute("v").value(), "по умолчанию");
    }

    // Внешняя сущность подставляется как разметка
    // An external entity is substituted as markup
    void testExternalEntity()
    {
        std::filesystem::path root = support::makeTestDirectory("external_entity");
        support::writeTestFile(root / "part.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?><p>часть</p>");
        support::writeTestFile(root / "main.xml",
            "<!DOCTYPE r [<!ENTITY part SYSTEM \"part.xml\">]><r>&part;</r>");
        beresta::FileResourceResolver resolver(root);
        beresta::ParserOptions options;
        options.resolver = &resolver;

        beresta::Document document = beresta::XmlParser(options).parseFile(root / "main.xml");
        TEST_ASSERT_EQ(document.toString(), "<!DOCTYPE r[<!ENTITY part SYSTEM \"part.xml\">]><r><p>часть</p></r>");
    }

    // Без резолвера внешнее не загружается; выход за корень и сеть запрещены
    // Without a resolver nothing external is loaded; leaving the root and the network are forbidden
    void testResourceSecurity()
    {
        std::filesystem::path root = support::makeTestDirectory("security");
        beresta::FileResourceResolver resolver(root);
        beresta::ResourceRequest outside{beresta::ResourceKind::EXTERNAL_ENTITY, "", "../secret.txt", ""};
        beresta::ResourceRequest remote{beresta::ResourceKind::EXTERNAL_ENTITY, "", "http://evil/x", ""};
        const char* xml = "<!DOCTYPE r [<!ENTITY x SYSTEM \"x.xml\">]><r>&x;</r>";
        beresta::DomErrorCode code = beresta::DomErrorCode::NOT_FOUND;

        TEST_ASSERT(!resolver.resolve(outside).has_value());
        TEST_ASSERT(!resolver.resolve(remote).has_value());
        try
        {
            beresta::XmlParser().parse(xml);
        }
        catch (const beresta::ParseException& error)
        {
            code = error.getCode();
        }
        TEST_ASSERT(code == beresta::DomErrorCode::RESOURCE_ERROR);
    }
}

namespace suites
{
    // Регистрация тестов внешних ресурсов
    // Registers the external resource tests
    void registerExternalResourceTests(test::TestRunner& runner)
    {
        runner.add("ресурсы: внешний DTD", testExternalDtd);
        runner.add("ресурсы: внешняя сущность", testExternalEntity);
        runner.add("ресурсы: безопасность загрузки", testResourceSecurity);
    }
}
