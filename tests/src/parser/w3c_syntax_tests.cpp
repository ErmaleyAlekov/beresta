#include "suites/test_suites.hpp"
#include "support/parse_helpers.hpp"

/*
** Регрессионные тесты расхождений с W3C XML Conformance Test Suite,
** найденных прогонщиком tests/conformance: синтаксис документа.
** В комментарии к каждой проверке — ID случая из набора W3C.
**
** Regression tests for deviations from the W3C XML Conformance Test Suite
** found by the tests/conformance runner: the document syntax.
** The comment of each check gives the case ID from the W3C suite.
*/
namespace
{
    // XML-декларация без encoding: пробел перед standalone не должен «съедаться» (sa01, o-p32pass1)
    // An XML declaration without encoding: the space before standalone must not be "eaten" (sa01, o-p32pass1)
    void testDeclarationWithoutEncoding()
    {
        beresta::Document document = beresta::XmlParser().parse("<?xml version='1.0' standalone='yes'?><a/>");

        TEST_ASSERT(document.getDeclaration().has_value());
        TEST_ASSERT(document.getDeclaration()->standalone == true);
        TEST_ASSERT(document.getDeclaration()->encoding.empty());
        TEST_ASSERT(support::errorCodeOf("<?xml version=\"1.0\"standalone=\"yes\"?><a/>")
            == beresta::DomErrorCode::PARSE_ERROR);
    }

    // Концы строк нормализуются на входе, а \r из &#13; сохраняется (valid-sa-068, -110, -116, -017)
    // Line endings are normalized on input, while \r from &#13; is kept (valid-sa-068, -110, -116, -017)
    void testLineEndsAndCharacterReferences()
    {
        beresta::XmlParser parser;
        beresta::Document entity = parser.parse("<!DOCTYPE d [<!ENTITY e \"&#13;\">]><d>x&e;y</d>");
        beresta::Document cdata = parser.parse("<d><![CDATA[a\r\nb\rc]]></d>");
        beresta::Document attribute = parser.parse(
            "<!DOCTYPE d [<!ENTITY e \"&#13;&#10;\">]><d a=\"x&e;y\" b=\"p\r\nq\"/>");
        beresta::Document instruction = parser.parse("<d><?pi data  ?></d>");
        const beresta::Node* pi = instruction.getRoot()->getChild(0);

        TEST_ASSERT_EQ(entity.getRoot()->getTextContent(), "x\ry");
        TEST_ASSERT_EQ(cdata.getRoot()->getTextContent(), "a\nb\nc");
        TEST_ASSERT_EQ(attribute.getRoot()->getAttribute("a").value(), "x  y");
        TEST_ASSERT_EQ(attribute.getRoot()->getAttribute("b").value(), "p q");
        TEST_ASSERT(pi->getType() == beresta::NodeType::PROCESSING_INSTRUCTION);
        TEST_ASSERT_EQ(static_cast<const beresta::ProcessingInstruction*>(pi)->getData(), "data  ");
    }

    // Символы имён по таблицам 5-й редакции: × (U+00D7) не буква, · (U+00B7) — только внутри имени
    // Name characters by the 5th edition tables: × (U+00D7) is not a letter, · (U+00B7) only inside a name
    void testNameCharacters()
    {
        beresta::XmlParser parser;

        TEST_ASSERT(support::errorCodeOf("<\xC3\x97/>") == beresta::DomErrorCode::PARSE_ERROR);
        TEST_ASSERT(support::errorCodeOf("<\xC2\xB7/>") == beresta::DomErrorCode::PARSE_ERROR);
        TEST_ASSERT_EQ(parser.parse("<a\xC2\xB7" "b/>").getRoot()->getName(), "a\xC2\xB7" "b");
        TEST_ASSERT_EQ(parser.parse("<элемент атрибут=\"1\"/>").getRoot()->getName(), "элемент");
        TEST_ASSERT_THROWS(beresta::Element("\xC3\x97"), beresta::DomException);
        TEST_ASSERT(support::errorCodeOf("<!DOCTYPE d [<?\xC3\x97 x?>]><d/>")
            == beresta::DomErrorCode::PARSE_ERROR);
    }

    // Разметка DTD и пространства имён (pubid01, not-wf-sa-149, ibm85n01, rmt-ns10-042..044, NE13c)
    // DTD markup and namespaces (pubid01, not-wf-sa-149, ibm85n01, rmt-ns10-042..044, NE13c)
    void testDtdMarkupAndNamespaces()
    {
        const char* const rejected[] = {
            "<!DOCTYPE d [<!ENTITY e PUBLIC \"a{b\" \"x\">]><d/>",
            "<!DOCTYPE d [<?xml version=\"1.0\"?>]><d/>",
            "<!DOCTYPE d [<!-- a -- b -->]><d/>",
            "<!DOCTYPE d [<?bad+ x?>]><d/>",
            "<?a:b x?><d/>",
            "<!DOCTYPE d [<!ENTITY a:b \"x\">]><d/>",
            "<!DOCTYPE d [<!NOTATION a:b SYSTEM \"n\">]><d/>",
            "<xmlns:a/>"
        };
        beresta::ParserOptions plain;

        for (const char* document : rejected)
            TEST_ASSERT(support::errorCodeOf(document) != beresta::DomErrorCode::NOT_FOUND);
        plain.namespaceAware = false;
        TEST_ASSERT(beresta::XmlParser(plain).parse("<?a:b x?><d/>").getRoot() != nullptr);
        TEST_ASSERT(beresta::XmlParser().parse("<!DOCTYPE d [<!ENTITY e SYSTEM \"\">]><d/>").getRoot()
            != nullptr);
    }
}

namespace suites
{
    // Регистрация тестов синтаксиса, найденных набором W3C
    // Registers the syntax tests found by the W3C suite
    void registerW3cSyntaxTests(test::TestRunner& runner)
    {
        runner.add("W3C: XML-декларация без encoding", testDeclarationWithoutEncoding);
        runner.add("W3C: концы строк и &#13;", testLineEndsAndCharacterReferences);
        runner.add("W3C: символы имён 5-й редакции", testNameCharacters);
        runner.add("W3C: разметка DTD и пространства имён", testDtdMarkupAndNamespaces);
    }
}
