#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "support/parse_helpers.hpp"

/*
** Регрессии ревью кода 2026-11-21 (задачи Т-6…Т-8): вложенность условных
** секций DTD, неверное значение ID, пустые повторы с огромным minOccurs
** и xs:all больше 64 частиц.
**
** Regressions of the code review of 2026-11-21 (tasks T-6…T-8): the nesting of DTD
** conditional sections, an invalid ID value, empty repeats with a huge minOccurs
** and xs:all with more than 64 particles.
*/
namespace
{
    /*
    ** Внешний DTD из depth вложенных <![INCLUDE[: глубже 64 — ошибка предела,
    ** а не переполнение стека (Т-6).
    **
    ** An external DTD of depth nested <![INCLUDE[: deeper than 64 is a limit error,
    ** not a stack overflow (T-6).
    */
    void testConditionalDepth()
    {
        const support::TestDirectory temporary("review_conditional");
        const auto& root = temporary.path;
        beresta::FileResourceResolver resolver(root);
        beresta::ParserOptions options;
        options.resolver = &resolver;
        options.loadExternalDtd = true;
        std::string shallow;
        std::string deep;

        for (int index = 0; index < 10; ++index)
            shallow = "<![INCLUDE[" + shallow + "]]>";
        for (int index = 0; index < 5000; ++index)
            deep += "<![INCLUDE[";
        for (int index = 0; index < 5000; ++index)
            deep += "]]>";
        support::writeTestFile(root / "shallow.dtd", "<!ELEMENT r ANY>" + shallow);
        support::writeTestFile(root / "deep.dtd", "<!ELEMENT r ANY>" + deep);
        support::writeTestFile(root / "shallow.xml", "<!DOCTYPE r SYSTEM \"shallow.dtd\"><r/>");
        support::writeTestFile(root / "deep.xml", "<!DOCTYPE r SYSTEM \"deep.dtd\"><r/>");
        TEST_ASSERT_EQ(beresta::XmlParser(options).parseFile(root / "shallow.xml").toString(),
            std::string("<!DOCTYPE r SYSTEM \"shallow.dtd\"><r/>"));
        TEST_ASSERT_THROWS(beresta::XmlParser(options).parseFile(root / "deep.xml"), beresta::DomException);
    }

    /*
    ** id="a b" у атрибута типа ID — одна ошибка значения, и "b" не становится
    ** идентификатором: ссылка на него — вторая ошибка (Т-7).
    **
    ** id="a b" of an ID attribute is one value error, and "b" does not become
    ** an identifier: a reference to it is the second error (T-7).
    */
    void testInvalidIdNotRegistered()
    {
        const char* dtd = "<!DOCTYPE r [<!ELEMENT r (e, ref)><!ELEMENT e EMPTY><!ELEMENT ref EMPTY>"
            "<!ATTLIST e id ID #IMPLIED><!ATTLIST ref to IDREF #IMPLIED>]>";

        TEST_ASSERT(support::countValidityErrors(std::string(dtd) + "<r><e id=\"a b\"/><ref to=\"b\"/></r>") == 2);
        TEST_ASSERT(support::countValidityErrors(std::string(dtd) + "<r><e id=\"b\"/><ref to=\"b\"/></r>") == 0);
    }

    /*
    ** minOccurs="1000000000" у частицы, совпадающей с пустотой, внутри повторяемой
    ** группы проверяется сразу, а не миллиардом пустых шагов (Т-8).
    **
    ** minOccurs="1000000000" on a particle matching emptiness inside a repeatable
    ** group is checked at once, not by a billion empty steps (T-8).
    */
    void testHugeEmptyRepeats()
    {
        std::shared_ptr<const beresta::xsd::Schema> schema = support::schemaFromBody(
            "<xs:element name=\"r\"><xs:complexType><xs:sequence maxOccurs=\"unbounded\">"
            "<xs:sequence minOccurs=\"1000000000\" maxOccurs=\"1000000000\"><xs:element name=\"a\" minOccurs=\"0\"/>"
            "</xs:sequence></xs:sequence></xs:complexType></xs:element>");
        beresta::Document document = beresta::XmlParser().parse("<r><a/><a/><a/></r>");
        beresta::ParserOptions options;
        options.schema = schema;

        TEST_ASSERT(beresta::xsd::SchemaValidator(schema).validate(document).empty());
        TEST_ASSERT_EQ(beresta::XmlParser(options).parse("<r><a/><a/><a/></r>").toString(),
            std::string("<r><a/><a/><a/></r>"));
    }

    /*
    ** xs:all из 64 частиц работает целиком, из 65 — честная ошибка схемы: раньше
    ** частицы после 64-й молча считались недопустимыми (и был сдвиг на 64 бита, Т-8).
    **
    ** xs:all of 64 particles works fully, of 65 is an honest schema error: previously
    ** the particles past the 64th were silently treated as not allowed (and there was a shift by 64 bits, T-8).
    */
    void testLargeAllGroup()
    {
        std::string particles;

        for (int index = 0; index < 64; ++index)
            particles += "<xs:element name=\"e" + std::to_string(index) + "\" minOccurs=\"0\"/>";
        std::string body = "<xs:element name=\"r\"><xs:complexType><xs:all>" + particles;
        TEST_ASSERT(support::schemaErrors("<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">" + body
            + "</xs:all></xs:complexType></xs:element></xs:schema>", "<r><e63/><e0/></r>").empty());
        TEST_ASSERT_THROWS(support::schemaFromBody(body + "<xs:element name=\"e64\"/></xs:all></xs:complexType>"
            "</xs:element>"), beresta::DomException);
    }
}

namespace suites
{
    // Регистрация регрессий ревью: DTD и модели содержимого
    // Registers the review regressions: DTD and content models
    void registerReviewDtdContentTests(test::TestRunner& runner)
    {
        runner.add("Ревью Т-6: вложенность условных секций", testConditionalDepth);
        runner.add("Ревью Т-7: неверное значение ID не регистрируется", testInvalidIdNotRegistered);
        runner.add("Ревью Т-8: огромный minOccurs у пустых повторов", testHugeEmptyRepeats);
        runner.add("Ревью Т-8: xs:all больше 64 частиц", testLargeAllGroup);
    }
}
