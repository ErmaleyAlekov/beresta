#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

namespace
{
    // Документ с пространством по умолчанию, префиксами, переопределением и отменой
    // A document with a default namespace, prefixes, a redefinition and a cancellation
    const char* const NAMESPACED =
        "<root xmlns=\"urn:d\" xmlns:p=\"urn:p\" a=\"1\">"
        "<inner xmlns:p=\"urn:p2\" xmlns:q=\"urn:q\"><leaf xmlns=\"\"/></inner></root>";

    // Строковый результат выражения по документу
    // The string result of an expression over the document
    std::string evaluate(const beresta::Document& document, const std::string& expression)
    {
        return beresta::xpath::evaluateString(document, expression);
    }

    /*
    ** Состав оси namespace: у корня — по умолчанию, p и неявный xml; у inner
    ** p переопределён, добавлен q; у leaf пространство по умолчанию
    ** отменено (xmlns=""), поэтому узла для него нет. Объявления xmlns
    ** не являются атрибутами: count(@*) у корня — 1.
    **
    ** The content of the namespace axis: the root has the default one, p and the implicit xml; inner
    ** has p redefined and q added; leaf has the default namespace
    ** cancelled (xmlns=""), so there is no node for it. xmlns declarations
    ** are not attributes: count(@*) of the root is 1.
    */
    void testNamespaceAxisContents()
    {
        beresta::Document document = beresta::XmlParser().parse(NAMESPACED);

        TEST_ASSERT_EQ(evaluate(document, "count(/*/namespace::*)"), "3");
        TEST_ASSERT_EQ(evaluate(document, "count(/*/*/namespace::*)"), "4");
        TEST_ASSERT_EQ(evaluate(document, "count(//*[local-name()='leaf']/namespace::*)"), "3");
        TEST_ASSERT_EQ(evaluate(document, "string(/*/*/namespace::p)"), "urn:p2");
        TEST_ASSERT_EQ(evaluate(document, "string(/*/namespace::xml)"), "http://www.w3.org/XML/1998/namespace");
        TEST_ASSERT_EQ(evaluate(document, "count(/*/@*)"), "1");
    }

    /*
    ** Имя узла пространства имён — префикс (у пространства по умолчанию —
    ** пустая строка), namespace-uri() пуст, родитель — элемент-владелец,
    ** проверки text()/comment() узлы пространств имён не выбирают.
    **
    ** The name of a namespace node is the prefix (for the default namespace —
    ** an empty string), namespace-uri() is empty, the parent is the owner element,
    ** the text()/comment() tests do not select namespace nodes.
    */
    void testNamespaceNodeProperties()
    {
        beresta::Document document = beresta::XmlParser().parse(NAMESPACED);

        TEST_ASSERT_EQ(evaluate(document, "name(/*/*/namespace::q)"), "q");
        TEST_ASSERT_EQ(evaluate(document, "local-name(/*/*/namespace::q)"), "q");
        TEST_ASSERT_EQ(evaluate(document, "namespace-uri(/*/*/namespace::q)"), "");
        TEST_ASSERT_EQ(evaluate(document, "count(/*/namespace::*[name()=''])"), "1");
        TEST_ASSERT_EQ(evaluate(document, "local-name(/*/*/namespace::q/..)"), "inner");
        TEST_ASSERT_EQ(evaluate(document, "count(/*/namespace::node())"), "3");
        TEST_ASSERT_EQ(evaluate(document, "count(/*/namespace::text())"), "0");
    }

    /*
    ** Множества узлов: объединение без дубликатов, порядок документа (узлы
    ** пространств имён — после элемента, до его атрибутов), p:* на оси
    ** namespace не выбирает ничего (у имени такого узла нет URI).
    **
    ** Node sets: a union without duplicates, document order (namespace
    ** nodes come after the element, before its attributes), p:* on the
    ** namespace axis selects nothing (the name of such a node has no URI).
    */
    void testNamespaceNodeSets()
    {
        beresta::Document document = beresta::XmlParser().parse(NAMESPACED);
        beresta::xpath::XPathContext context;
        context.namespaces.emplace("d", "urn:d");

        TEST_ASSERT_EQ(evaluate(document, "count(/*/namespace::* | /*/namespace::p)"), "3");
        TEST_ASSERT_EQ(evaluate(document, "name((/*/@a | /*/namespace::p)[1])"), "p");
        TEST_ASSERT_EQ(beresta::xpath::XPathExpression("count(/d:root/namespace::d:*)").evaluate(document, context)
            .toString(), "0");
    }
}

namespace suites
{
    // Регистрация тестов оси namespace
    // Registers the namespace axis tests
    void registerXPathNamespaceTests(test::TestRunner& runner)
    {
        runner.add("XPath: состав оси namespace", testNamespaceAxisContents);
        runner.add("XPath: имя, URI и родитель узла пространства имён", testNamespaceNodeProperties);
        runner.add("XPath: узлы пространств имён во множествах", testNamespaceNodeSets);
    }
}
