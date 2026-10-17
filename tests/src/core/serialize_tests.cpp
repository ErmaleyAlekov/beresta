#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

#include <memory>

namespace
{
    // Спецсимволы экранируются при выводе и восстанавливаются при разборе
    void testEscapingRoundTrip()
    {
        beresta::Element element("e");
        element.setAttribute("v", "a&b<c>\"d\"\te\nf");
        element.appendChild(std::make_unique<beresta::Text>("1 < 2 & 3 > 2"));
        std::string xml = element.toString();

        TEST_ASSERT_EQ(xml, "<e v=\"a&amp;b&lt;c&gt;&quot;d&quot;&#9;e&#10;f\">"
            "1 &lt; 2 &amp; 3 &gt; 2</e>");
        std::unique_ptr<beresta::Element> parsed = beresta::XmlParser().parseElement(xml);
        TEST_ASSERT_EQ(parsed->getAttribute("v").value(), "a&b<c>\"d\"\te\nf");
        TEST_ASSERT_EQ(parsed->getTextContent(), "1 < 2 & 3 > 2");
    }

    // Пустой элемент выводится как <x/>, с детьми — как <x>...</x>
    void testEmptyAndNonEmptyElements()
    {
        beresta::Element element("x");

        TEST_ASSERT_EQ(element.toString(), "<x/>");
        element.appendChild(std::make_unique<beresta::Text>(""));
        TEST_ASSERT_EQ(element.toString(), "<x></x>");
    }

    // Содержимое, которое сломало бы синтаксис вывода, отвергается
    void testInvalidContentRejected()
    {
        beresta::Comment comment("ok");

        TEST_ASSERT_THROWS(beresta::Comment("a--b"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::Comment("tail-"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::CDataSection("x]]>y"), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::ProcessingInstruction("pi", "a?>b"), beresta::DomException);
        TEST_ASSERT_THROWS(comment.setValue("--"), beresta::DomException);
        TEST_ASSERT_EQ(comment.getValue(), "ok");
    }

    // Инструкция обработки: пробел только между целью и непустыми данными
    void testProcessingInstructionFormat()
    {
        beresta::ProcessingInstruction bare("target");
        beresta::ProcessingInstruction full("target", "a=\"1\"");

        TEST_ASSERT_EQ(bare.toString(), "<?target?>");
        TEST_ASSERT_EQ(full.toString(), "<?target a=\"1\"?>");
    }
}

namespace suites
{
    // Регистрация тестов сериализации
    void registerSerializeTests(test::TestRunner& runner)
    {
        runner.add("вывод: экранирование и обратный разбор", testEscapingRoundTrip);
        runner.add("вывод: пустые и непустые элементы", testEmptyAndNonEmptyElements);
        runner.add("вывод: недопустимое содержимое", testInvalidContentRejected);
        runner.add("вывод: формат инструкции обработки", testProcessingInstructionFormat);
    }
}
