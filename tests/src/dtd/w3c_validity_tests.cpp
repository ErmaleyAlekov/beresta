#include "suites/test_suites.hpp"
#include "support/parse_helpers.hpp"
#include "support/resource_helpers.hpp"

/*
** Регрессионные тесты расхождений с набором W3C: ограничения
** валидности (VC), которые библиотека раньше не проверяла.
**
** Regression tests for deviations from the W3C suite: validity
** constraints (VC) the library did not check before.
*/
namespace
{
    // Ограничения на сами объявления (el04, dtd01, id03, id05, attr04, attr11, attr15, E2a, E06*, E9b)
    // Constraints on the declarations themselves (el04, dtd01, id03, id05, attr04, attr11, attr15, E2a, E06*, E9b)
    void testDeclarationConstraints()
    {
        const char* const invalid[] = {
            "<!DOCTYPE d [<!ELEMENT d ANY><!ELEMENT d EMPTY>]><d/>",
            "<!DOCTYPE d [<!ELEMENT d (#PCDATA|x|x)*><!ELEMENT x EMPTY>]><d/>",
            "<!DOCTYPE d [<!ELEMENT d EMPTY><!ATTLIST d a ID #IMPLIED b ID #IMPLIED>]><d/>",
            "<!DOCTYPE d [<!ELEMENT d EMPTY><!ATTLIST d a ID \"x\">]><d/>",
            "<!DOCTYPE d [<!ELEMENT d EMPTY><!ATTLIST d a (x|x) #IMPLIED>]><d/>",
            "<!DOCTYPE d [<!ELEMENT d ANY><!ATTLIST d a NOTATION (n) #IMPLIED>]><d/>",
            "<!DOCTYPE d [<!ELEMENT d ANY><!ENTITY u SYSTEM \"u\" NDATA n>]><d/>",
            "<!DOCTYPE d [<!ELEMENT d ANY><!ATTLIST d a NMTOKEN \"@\">]><d/>",
            "<!DOCTYPE d [<!ELEMENT d ANY><!ATTLIST d a (x|y) \"z\">]><d/>",
            "<!DOCTYPE d [<!ELEMENT d ANY><!ATTLIST d a IDREF \"1x\">]><d/>"
        };

        for (const char* document : invalid)
            TEST_ASSERT(support::countValidityErrors(document) >= 1);
        TEST_ASSERT(support::countValidityErrors(
            "<!DOCTYPE d [<!ELEMENT d ANY><!ATTLIST d a (x|y) \"x\" b NMTOKENS \"p q\">]><d/>") == 0);
    }

    // EMPTY не допускает даже комментария, PI и пустой сущности; списки делятся только пробелами
    // EMPTY does not allow even a comment, a PI or an empty entity; lists are split only by spaces
    void testContentConstraints()
    {
        const char* const invalid[] = {
            "<!DOCTYPE d [<!ELEMENT d EMPTY>]><d><!--c--></d>",
            "<!DOCTYPE d [<!ELEMENT d EMPTY>]><d><?p x?></d>",
            "<!DOCTYPE d [<!ELEMENT d EMPTY><!ENTITY e \"\">]><d>&e;</d>",
            "<!DOCTYPE d [<!ELEMENT d EMPTY><!ATTLIST d a NMTOKENS #IMPLIED>]><d a=\"x&#9;y\"/>",
            "<!DOCTYPE d [<!ELEMENT d EMPTY><!ATTLIST d a ID #IMPLIED>]><d a=\"p:q\"/>"
        };

        for (const char* document : invalid)
            TEST_ASSERT(support::countValidityErrors(document) >= 1);
        TEST_ASSERT(support::countValidityErrors(
            "<!DOCTYPE d [<!ELEMENT d EMPTY><!ATTLIST d a NMTOKENS #IMPLIED>]><d a=\" x  y \"/>") == 0);
    }

    // VC: Standalone Document Declaration — результат не должен зависеть от внешнего DTD (not-sa*)
    // VC: Standalone Document Declaration — the result must not depend on the external DTD (not-sa*)
    void testStandaloneValidity()
    {
        support::FileMap files{{"d.dtd",
            "<!ELEMENT d (e)*><!ELEMENT e EMPTY><!ATTLIST e a CDATA \"v\" t NMTOKEN #IMPLIED>"}};
        std::string yes = "<?xml version=\"1.0\" standalone=\"yes\"?><!DOCTYPE d SYSTEM \"d.dtd\">";
        std::string no = "<?xml version=\"1.0\" standalone=\"no\"?><!DOCTYPE d SYSTEM \"d.dtd\">";

        TEST_ASSERT(support::parseWithFiles(yes + "<d><e a=\"v\"/></d>", files, true).validityErrors == 0);
        TEST_ASSERT(support::parseWithFiles(yes + "<d><e/></d>", files, true).validityErrors >= 1);
        TEST_ASSERT(support::parseWithFiles(yes + "<d> <e a=\"v\"/> </d>", files, true).validityErrors >= 1);
        TEST_ASSERT(support::parseWithFiles(yes + "<d><e a=\"v\" t=\" x \"/></d>", files, true)
            .validityErrors >= 1);
        TEST_ASSERT(support::parseWithFiles(no + "<d> <e/> </d>", files, true).validityErrors == 0);
    }

    // Скобки группы и конец объявления в разных PE — ошибка валидности (invalid--002, -005, ibm49i01)
    // Group brackets and the end of a declaration in different PEs are a validity error (invalid--002, -005, ibm49i01)
    void testParameterEntityNesting()
    {
        std::string document = "<!DOCTYPE d SYSTEM \"d.dtd\"><d/>";
        support::ParseOutcome group = support::parseWithFiles(document,
            {{"d.dtd", "<!ENTITY % open \"(#PCDATA\"><!ELEMENT d %open;)>"}}, true);
        support::ParseOutcome declaration = support::parseWithFiles(document,
            {{"d.dtd", "<!ENTITY % close \">\"><!ELEMENT d EMPTY %close;"}}, true);

        TEST_ASSERT(!group.fatal && group.validityErrors >= 1);
        TEST_ASSERT(!declaration.fatal && declaration.validityErrors >= 1);
        TEST_ASSERT(support::parseWithFiles(document,
            {{"d.dtd", "<!ENTITY % m \"(#PCDATA)\"><!ELEMENT d %m;>"}}, true).validityErrors == 0);
    }
}

namespace suites
{
    // Регистрация тестов ограничений валидности, найденных набором W3C
    // Registers the validity constraint tests found by the W3C suite
    void registerW3cValidityTests(test::TestRunner& runner)
    {
        runner.add("W3C: ограничения объявлений DTD", testDeclarationConstraints);
        runner.add("W3C: ограничения содержимого", testContentConstraints);
        runner.add("W3C: standalone и валидность", testStandaloneValidity);
        runner.add("W3C: вложенность параметрических сущностей", testParameterEntityNesting);
    }
}
