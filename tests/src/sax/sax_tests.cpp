#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "support/recording_handler.hpp"

namespace
{
    // Порядок и содержимое событий
    void testEventSequence()
    {
        support::RecordingHandler handler;

        beresta::SaxParser().parse(
            "<?xml version=\"1.0\"?><a x=\"1\">t<!--c--><![CDATA[d]]><?p q?><b/></a>", handler);
        TEST_ASSERT_EQ(handler.getLog(),
            "begin|decl:1.0|start:a[x=1]|text:t|comment:c|cdata:d|pi:p=q|start:b[]|end:b|end:a|finish");
    }

    // Пробелы между тегами приходят отдельным событием и не смешиваются с текстом
    void testIgnorableWhitespace()
    {
        support::RecordingHandler handler;

        beresta::SaxParser().parse("<a>\n  <b>x</b>\n</a>", handler);
        TEST_ASSERT_EQ(handler.getLog(), "begin|start:a[]|ws|start:b[]|text:x|end:b|ws|end:a|finish");
    }

    // Пространство имён элемента и атрибутов доступно прямо в событии
    void testNamespaceEvents()
    {
        support::RecordingHandler handler;

        beresta::SaxParser().parse("<n:a xmlns:n=\"urn:n\" n:k=\"v\"/>", handler);
        TEST_ASSERT_EQ(handler.getLastNamespace(), "urn:n");
        TEST_ASSERT_EQ(handler.getLastAttributeNamespace(), "urn:n");
    }

    // trackLocations записывает строку и столбец каждого узла в DOM
    void testDomLocations()
    {
        beresta::ParserOptions options;
        options.trackLocations = true;
        beresta::Document document = beresta::XmlParser(options).parse("<r>\n  <item/>\n</r>");
        const beresta::Element* item = document.getRoot()->findChildElement("item");

        TEST_ASSERT(item->getSourceLocation().line == 2);
        TEST_ASSERT(item->getSourceLocation().column == 3);
        TEST_ASSERT(document.getRoot()->getSourceLocation().line == 1);
    }
}

namespace suites
{
    // Регистрация тестов потокового разбора
    void registerSaxTests(test::TestRunner& runner)
    {
        runner.add("SAX: последовательность событий", testEventSequence);
        runner.add("SAX: пробельное форматирование", testIgnorableWhitespace);
        runner.add("SAX: пространства имён в событиях", testNamespaceEvents);
        runner.add("SAX: позиции узлов в DOM", testDomLocations);
    }
}
