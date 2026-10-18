#include "fuzz/fuzz_input.hpp"

#include "beresta.hpp"

#include <optional>

/*
** Цель фаззинга: XPathExpression — компиляция и вычисление.
**
** Формат входа: [байт флагов][выражение] 0xF5 0xF5 [документ].
** Если документ не задан или не разобрался, берётся встроенный документ,
** где есть все виды узлов: элементы с пространствами имён, атрибуты,
** ID из DTD, текст, CDATA, комментарий и PI.
** Флаг 0x01 выбирает контекстный узел: корневой элемент вместо документа.
**
** The fuzzing target: XPathExpression — compilation and evaluation.
** The input format: [flag byte][expression] 0xF5 0xF5 [document].
** If the document is not given or did not parse, a built-in document is used
** that has all node kinds: elements with namespaces, attributes,
** IDs from a DTD, text, CDATA, a comment and a PI.
** The 0x01 flag chooses the context node: the root element instead of the document.
*/
namespace
{
    const char* const DEFAULT_DOCUMENT =
        "<?xml version=\"1.0\"?>\n"
        "<!DOCTYPE r [<!ATTLIST e id ID #IMPLIED>]>\n"
        "<r xmlns:p=\"urn:p\" xmlns=\"urn:d\" a=\"1\">"
        "<e id=\"x1\" p:b=\"2\">text<![CDATA[cd]]><!--c--><?pi d?></e>"
        "<p:e id=\"x2\">3.5</p:e><e xml:lang=\"en-US\"/>tail</r>";

    // Документ из входа или встроенный (если вход пуст или некорректен)
    // The document from the input or the built-in one (if the input is empty or invalid)
    beresta::Document loadDocument(std::string_view text)
    {
        beresta::XmlParser parser;

        if (!text.empty())
        {
            try
            {
                return parser.parse(text);
            }
            catch (const beresta::DomException&)
            {
            }
        }
        return parser.parse(DEFAULT_DOCUMENT);
    }

    // Префиксы и переменные, на которые может сослаться выражение
    // The prefixes and variables the expression may refer to
    beresta::xpath::XPathContext makeContext(const beresta::Document& document)
    {
        beresta::xpath::XPathContext context;

        context.namespaces["p"] = "urn:p";
        context.namespaces["d"] = "urn:d";
        context.variables.emplace("s", beresta::xpath::XPathValue::fromString("x1 x2"));
        context.variables.emplace("n", beresta::xpath::XPathValue::fromNumber(2.0));
        context.variables.emplace("b", beresta::xpath::XPathValue::fromBoolean(true));
        context.variables.emplace("all", beresta::xpath::XPathExpression("//node()").evaluate(document));
        return context;
    }

    // Вычисление и все преобразования результата (каждое может споткнуться по-своему)
    // Evaluation and all conversions of the result (each may stumble in its own way)
    void evaluate(const beresta::xpath::XPathExpression& expression, const beresta::Node& node,
        const beresta::xpath::XPathContext& context)
    {
        beresta::xpath::XPathValue value = expression.evaluate(node, context);

        (void)value.toString();
        (void)value.toNumber();
        (void)value.toBoolean();
        for (const beresta::xpath::XPathItem& item : value.getNodes())
            (void)beresta::xpath::stringValue(item);
    }
}

// Точка входа libFuzzer
// The libFuzzer entry point
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    if (size == 0)
        return 0;
    std::string_view input(reinterpret_cast<const char*>(data + 1), size - 1);
    std::vector<std::string_view> parts = fuzz::splitParts(input, 2);
    std::string_view documentText;

    if (parts.size() > 1)
        documentText = parts[1];
    beresta::Document document = loadDocument(documentText);
    beresta::xpath::XPathContext context = makeContext(document);
    const beresta::Node* node = &document;

    if ((data[0] & 0x01) != 0 && document.getRoot() != nullptr)
        node = document.getRoot();
    try
    {
        beresta::xpath::XPathExpression expression(parts[0]);

        evaluate(expression, *node, context);
    }
    catch (const beresta::DomException&)
    {
        // Синтаксическая или вычислительная ошибка XPath — штатный исход
        // An XPath syntax or evaluation error is a regular outcome
    }
    return 0;
}
