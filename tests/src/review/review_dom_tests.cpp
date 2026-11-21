#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "utils/decimal.hpp"
#include "utils/qualified_name.hpp"

#include <cmath>

/*
** Регрессии ревью кода 2026-11-21 (задачи Т-10, Т-11, Т-13): объявления xmlns
** через setAttributeNS, xml:space в красивом выводе, внутреннее подмножество
** DOCTYPE и перевод длинной десятичной записи в double.
**
** Regressions of the code review of 2026-11-21 (tasks T-10, T-11, T-13): xmlns declarations
** through setAttributeNS, xml:space in pretty output, the DOCTYPE internal
** subset and the conversion of a long decimal notation to double.
*/
namespace
{
    /*
    ** xmlns:p через setAttributeNS проходит те же проверки, что declareNamespace:
    ** пустой URI у префикса и чужое пространство запрещены, правильное объявление работает (Т-10).
    **
    ** xmlns:p through setAttributeNS passes the same checks as declareNamespace:
    ** an empty URI for a prefix and a foreign namespace are forbidden, a correct declaration works (T-10).
    */
    void testSetAttributeNsDeclarations()
    {
        beresta::Element element("e");
        std::string xmlns(beresta::XMLNS_NAMESPACE_URI);

        TEST_ASSERT_THROWS(element.setAttributeNS(xmlns, "xmlns:p", ""), beresta::DomException);
        TEST_ASSERT_THROWS(element.setAttributeNS("urn:other", "xmlns:p", "urn:p"), beresta::DomException);
        TEST_ASSERT_THROWS(element.setAttributeNS(xmlns, "a:b", "v"), beresta::DomException);
        TEST_ASSERT_THROWS(element.setAttributeNS(xmlns, "xmlns:xml", "urn:x"), beresta::DomException);
        TEST_ASSERT(!element.hasAttribute("xmlns:p"));
        element.setAttributeNS(xmlns, "xmlns:p", "urn:p");
        element.setAttributeNS(xmlns, "xmlns", "urn:default");
        TEST_ASSERT_EQ(element.getAttribute("xmlns:p").value_or(""), std::string("urn:p"));
        TEST_ASSERT_EQ(element.getAttribute("xmlns").value_or(""), std::string("urn:default"));
    }

    /*
    ** Под xml:space="preserve" (своим или унаследованным) красивый вывод не
    ** добавляет отступов: иначе повторный разбор дал бы новые текстовые узлы (Т-11).
    **
    ** Under xml:space="preserve" (own or inherited) pretty output does not
    ** add indentation: otherwise parsing again would give new text nodes (T-11).
    */
    void testPrettyPreservesSpace()
    {
        beresta::SerializerOptions options;
        options.prettyPrint = true;
        options.newline = "\n";
        options.indent = "  ";
        beresta::XmlSerializer serializer(options);
        beresta::Document document = beresta::XmlParser().parse(
            "<doc><pre xml:space=\"preserve\"><s>a</s><s>b</s></pre><p><q/></p></doc>");

        TEST_ASSERT_EQ(serializer.toString(*document.getRoot()),
            std::string("<doc>\n  <pre xml:space=\"preserve\"><s>a</s><s>b</s></pre>\n  <p>\n    <q/>\n  </p>\n</doc>"));
        TEST_ASSERT_EQ(serializer.toString(*static_cast<beresta::ContainerNode*>(document.getRoot()->getFirstChild())->getFirstChild()),
            std::string("<s>a</s>"));
        beresta::Document inner = beresta::XmlParser().parse("<r xml:space=\"preserve\"><a><b/></a></r>");
        TEST_ASSERT_EQ(serializer.toString(*inner.getRoot()->getFirstChild()), std::string("<a><b/></a>"));
    }

    /*
    ** Подмножество не может выйти за "[...]>" при выводе, публичный идентификатор —
    ** только PubidChar; «]» внутри литералов, комментариев и инструкций допустим (Т-11).
    **
    ** The subset cannot leave "[...]>" on output, the public identifier is
    ** PubidChar only; "]" inside literals, comments and instructions is allowed (T-11).
    */
    void testDocumentTypeSubset()
    {
        const char* subset = "<!ENTITY a \"]\"><!-- ] --><?pi ]?><!ATTLIST r x CDATA ']'>";
        beresta::Document parsed = beresta::XmlParser().parse(std::string("<!DOCTYPE r [") + subset + "]><r/>");

        TEST_ASSERT_THROWS(beresta::DocumentType("r", "", "", "]><injected/>"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::DocumentType("r", "", "", "<!ENTITY a \"x>"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::DocumentType("r", "", "", "<!-- x"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::DocumentType("r", "", "", "\"]\""), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::DocumentType("r", "-//A<B//EN", "a.dtd", ""), beresta::DomException);
        beresta::DocumentType type("r", "-//A//EN", "a.dtd", subset);
        TEST_ASSERT_EQ(beresta::XmlParser().parse(parsed.toString()).toString(), parsed.toString());
    }

    /*
    ** "0." и 400 единиц — около 1/9, а не NaN (на MSVC long double — это double,
    ** и 10^400 раньше переполнялось в INF / INF); то же в number() XPath (Т-13).
    **
    ** "0." and 400 ones is about 1/9, not NaN (on MSVC long double is double,
    ** and 10^400 used to overflow into INF / INF); the same in XPath number() (T-13).
    */
    void testLongDecimalToDouble()
    {
        beresta::utils::Decimal value;
        std::string ones(400, '1');
        beresta::Document document = beresta::XmlParser().parse("<r/>");

        TEST_ASSERT(beresta::utils::readDecimal("0." + ones, value));
        TEST_ASSERT(std::abs(beresta::utils::decimalToDouble(value) - 1.0 / 9.0) < 1e-15);
        TEST_ASSERT(beresta::utils::readDecimal(ones + "0000", value));
        TEST_ASSERT(std::isinf(beresta::utils::decimalToDouble(value)));
        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, "number('0." + ones + "') > 0.11"), std::string("true"));
        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, "number('12.5')"), std::string("12.5"));
    }
}

namespace suites
{
    // Регистрация регрессий ревью: DOM, сериализация и числа
    // Registers the review regressions: DOM, serialization and numbers
    void registerReviewDomTests(test::TestRunner& runner)
    {
        runner.add("Ревью Т-10: объявления xmlns через setAttributeNS", testSetAttributeNsDeclarations);
        runner.add("Ревью Т-11: xml:space в красивом выводе", testPrettyPreservesSpace);
        runner.add("Ревью Т-11: внутреннее подмножество DOCTYPE", testDocumentTypeSubset);
        runner.add("Ревью Т-13: длинная десятичная запись в double", testLongDecimalToDouble);
    }
}
