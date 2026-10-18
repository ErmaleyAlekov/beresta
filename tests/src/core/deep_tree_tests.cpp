#include "suites/test_suites.hpp"
#include "support/tree_helpers.hpp"

namespace
{
    /*
    ** Глубина, на которой рекурсивная реализация гарантированно падает:
    ** даже по 100 байт стека на уровень дали бы 10 МБ — больше стека
    ** главного потока и на Windows (1 МБ), и на Linux (8 МБ).
    **
    ** The depth at which a recursive implementation is guaranteed to crash:
    ** even 100 bytes of stack per level would give 10 MB — more than the main
    ** thread stack both on Windows (1 MB) and on Linux (8 MB).
    */
    const std::size_t DEEP = 100000;

    /*
    ** Копирование, сравнение, текст и разрушение глубокого дерева: каждая
    ** из этих операций раньше была рекурсивной и переполняла стек.
    **
    ** Copying, comparison, text and destruction of a deep tree: each
    ** of these operations used to be recursive and overflowed the stack.
    */
    void testDeepCopyCompareDestroy()
    {
        beresta::Document original = support::deepChain(DEEP);
        beresta::Document copy = original;

        TEST_ASSERT(original.isEqualNode(copy));
        TEST_ASSERT_EQ(copy.getRoot()->getTextContent(), "x");
        support::deepestElement(copy)->setTextContent("y");
        TEST_ASSERT(!original.isEqualNode(copy));
        std::unique_ptr<beresta::Node> subtree = original.releaseRoot();
        subtree.reset();
        TEST_ASSERT(original.getRoot() == nullptr);
    }

    /*
    ** Сериализация и обратный разбор глубокого документа, нормализация.
    ** Красивый вывод проверяется на меньшей глубине: суммарная длина
    ** отступов растёт как квадрат глубины (это свойство формата, не кода).
    **
    ** Serialization and parsing back of a deep document, normalization.
    ** Pretty output is checked at a smaller depth: the total length of the
    ** indentation grows as the square of the depth (a property of the format, not of the code).
    */
    void testDeepSerializeParseNormalize()
    {
        beresta::Document document = support::deepChain(DEEP);
        std::string text = document.toString();
        beresta::ParserOptions options;

        TEST_ASSERT(text.size() == DEEP * 7 + 1);
        options.maxDepth = DEEP + 1;
        beresta::Document parsed = beresta::XmlParser(options).parse(text);
        TEST_ASSERT(parsed.isEqualNode(document));
        support::deepestElement(parsed)->appendChild(std::make_unique<beresta::Text>(""));
        support::deepestElement(parsed)->appendChild(std::make_unique<beresta::Text>("z"));
        parsed.normalize();
        TEST_ASSERT_EQ(parsed.getRoot()->getTextContent(), "xz");
        beresta::SerializerOptions pretty;
        pretty.prettyPrint = true;
        std::string indented = beresta::XmlSerializer(pretty).toString(support::deepChain(3000));
        TEST_ASSERT(indented.find("\n" + std::string(2999 * 4, ' ') + "<a>x</a>") != std::string::npos);
    }

    // Проверка глубокого документа по рекурсивной схеме и XPath по нему
    // Validating a deep document against a recursive schema and XPath over it
    void testDeepValidationAndXPath()
    {
        const std::size_t depth = 5000;
        auto schema = beresta::xsd::Schema::fromString("<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
            "<xs:element name=\"a\"><xs:complexType mixed=\"true\"><xs:sequence><xs:element ref=\"a\" minOccurs=\"0\"/>"
            "</xs:sequence></xs:complexType></xs:element></xs:schema>");
        beresta::Document document = support::deepChain(depth);

        TEST_ASSERT(beresta::xsd::SchemaValidator(schema).isValid(document));
        support::deepestElement(document)->setAttribute("bad", "1");
        TEST_ASSERT(beresta::xsd::SchemaValidator(schema).validate(document).size() == 1);
        TEST_ASSERT_EQ(beresta::xpath::evaluateString(document, "count(//a)"), std::to_string(depth));
    }
}

namespace suites
{
    // Регистрация тестов глубоких деревьев (отсутствие рекурсии)
    // Registers the deep tree tests (absence of recursion)
    void registerDeepTreeTests(test::TestRunner& runner)
    {
        runner.add("Глубокое дерево: копирование, сравнение, разрушение", testDeepCopyCompareDestroy);
        runner.add("Глубокое дерево: вывод, разбор, нормализация", testDeepSerializeParseNormalize);
        runner.add("Глубокое дерево: XSD и XPath", testDeepValidationAndXPath);
    }
}
