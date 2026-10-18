#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "support/resource_helpers.hpp"

/*
** Регрессионные тесты расхождений с набором W3C: внешние сущности,
** текстовые декларации, standalone и необъявленные сущности.
**
** Regression tests for deviations from the W3C suite: external entities,
** text declarations, standalone and undeclared entities.
*/
namespace
{
    // Документ, подключающий внешнюю сущность e.xml
    // A document including the external entity e.xml
    const char* const ENTITY_DOCUMENT = "<!DOCTYPE d [<!ENTITY e SYSTEM \"e.xml\">]><d>&e;</d>";

    // Текстовая декларация: encoding обязателен, порядок строгий, standalone и 1.1 запрещены
    // The text declaration: encoding is mandatory, the order is strict, standalone and 1.1 are forbidden
    void testTextDeclarations()
    {
        const char* const rejected[] = {
            "<?xml version=\"1.0\"?>текст",
            "<?xml encoding=\"UTF-8\" version=\"1.0\"?>текст",
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>текст",
            "<?xml version=\"1.1\" encoding=\"UTF-8\"?>текст"
        };

        TEST_ASSERT_EQ(support::parseWithFiles(ENTITY_DOCUMENT,
            {{"e.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>текст"}}, false).rootText, "текст");
        TEST_ASSERT(!support::parseWithFiles(ENTITY_DOCUMENT, {{"e.xml", "<?xml encoding=\"UTF-8\"?>т"}},
            false).fatal);
        for (const char* entity : rejected)
            TEST_ASSERT(support::parseWithFiles(ENTITY_DOCUMENT, {{"e.xml", entity}}, false).fatal);
        TEST_ASSERT(support::parseWithFiles("<!DOCTYPE d SYSTEM \"d.dtd\"><d/>",
            {{"d.dtd", "<!ELEMENT d EMPTY><?xml version=\"1.0\" encoding=\"UTF-8\"?>"}}, false).fatal);
    }

    // standalone="yes" запрещает ссылки на сущности из внешнего DTD (not-sa03, ibm32n09, ibm68n06)
    // standalone="yes" forbids references to entities from the external DTD (not-sa03, ibm32n09, ibm68n06)
    void testStandaloneEntities()
    {
        support::FileMap files{{"d.dtd", "<!ENTITY ext \"x\">"}};
        std::string head = "<?xml version=\"1.0\" standalone=\"yes\"?><!DOCTYPE d SYSTEM \"d.dtd\">";
        std::string notStandalone = "<?xml version=\"1.0\" standalone=\"no\"?><!DOCTYPE d SYSTEM \"d.dtd\">";

        TEST_ASSERT(support::parseWithFiles(head + "<d>&ext;</d>", files, false).fatal);
        TEST_ASSERT(support::parseWithFiles(head + "<d a=\"&ext;\"/>", files, false).fatal);
        TEST_ASSERT_EQ(support::parseWithFiles(notStandalone + "<d>&ext;</d>", files, false).rootText, "x");
    }

    // При %pe; или внешнем DTD необъявленная сущность — ошибка валидности, а не синтаксиса (E13)
    // With %pe; or an external DTD an undeclared entity is a validity error, not a syntax one (E13)
    void testUndeclaredEntities()
    {
        std::string withReferences = "<!DOCTYPE d [<!ENTITY % p \"\"> %p;]><d>a&x;b</d>";
        support::ParseOutcome lenient = support::parseWithFiles(withReferences, {}, false);
        support::ParseOutcome validated = support::parseWithFiles(withReferences, {}, true);

        TEST_ASSERT(!lenient.fatal);
        TEST_ASSERT_EQ(lenient.rootText, "ab");
        TEST_ASSERT(!validated.fatal);
        TEST_ASSERT(validated.validityErrors >= 1);
        TEST_ASSERT(support::parseWithFiles("<!DOCTYPE d [<!ELEMENT d ANY>]><d>&x;</d>", {}, false).fatal);
        TEST_ASSERT(support::parseWithFiles("<?xml version=\"1.0\" standalone=\"yes\"?>" + withReferences,
            {}, false).fatal);
    }

    /*
    ** Внешняя PE во внутреннем подмножестве читается по правилам внешнего
    ** подмножества (E18, E60); PE между объявлениями обязана содержать их
    ** целиком (ibm28an01); systemId вложенной сущности — от её объявления (E18).
    **
    ** An external PE in the internal subset is read by the rules of the external
    ** subset (E18, E60); a PE between declarations must contain them whole
    ** (ibm28an01); the systemId of a nested entity is resolved from its declaration (E18).
    */
    void testExternalParameterEntities()
    {
        std::string document = "<!DOCTYPE d [<!ENTITY % e SYSTEM \"e.ent\"> %e;]><d/>";
        std::filesystem::path root = support::makeTestDirectory("w3c_entity_base");

        TEST_ASSERT(!support::parseWithFiles(document, {{"e.ent", "<![INCLUDE[<!ELEMENT d EMPTY>]]>"}},
            true).fatal);
        TEST_ASSERT(!support::parseWithFiles(document,
            {{"e.ent", "<!ENTITY % t \"EMPTY\"><!ELEMENT d %t;>"}}, true).fatal);
        TEST_ASSERT(support::parseWithFiles("<!DOCTYPE d SYSTEM \"d.dtd\"><d/>",
            {{"d.dtd", "<!ENTITY % s \"<!ELEMENT d \"> %s;EMPTY>"}}, false).fatal);
        std::filesystem::create_directories(root / "sub");
        support::writeTestFile(root / "sub" / "p.ent", "<!ENTITY t SYSTEM \"t.txt\">");
        support::writeTestFile(root / "sub" / "t.txt", "из подкаталога");
        support::writeTestFile(root / "doc.xml", "<!DOCTYPE d [<!ENTITY % p SYSTEM \"sub/p.ent\"> %p;]><d>&t;</d>");
        beresta::FileResourceResolver resolver(root);
        beresta::ParserOptions options;
        options.resolver = &resolver;
        TEST_ASSERT_EQ(beresta::XmlParser(options).parseFile(root / "doc.xml").getRoot()->getTextContent(),
            "из подкаталога");
    }
}

namespace suites
{
    // Регистрация тестов сущностей, найденных набором W3C
    // Registers the entity tests found by the W3C suite
    void registerW3cEntityTests(test::TestRunner& runner)
    {
        runner.add("W3C: текстовые декларации", testTextDeclarations);
        runner.add("W3C: standalone и внешние сущности", testStandaloneEntities);
        runner.add("W3C: необъявленные сущности при %pe;", testUndeclaredEntities);
        runner.add("W3C: внешние параметрические сущности", testExternalParameterEntities);
    }
}
