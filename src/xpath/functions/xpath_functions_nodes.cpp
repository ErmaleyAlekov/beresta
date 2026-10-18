#include "xpath/internal/xpath_eval.hpp"
#include "i18n/i18n.hpp"
#include "xpath/internal/xpath_functions.hpp"
#include "core/document.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xpath::detail
{
    // last(): размер контекстного множества
    // last(): the size of the context set
    XPathValue functionLast(const FunctionCall& call)
    {
        return XPathValue::fromNumber(static_cast<double>(call.context.size));
    }

    // position(): позиция контекстного узла
    // position(): the position of the context node
    XPathValue functionPosition(const FunctionCall& call)
    {
        return XPathValue::fromNumber(static_cast<double>(call.context.position));
    }

    // count(множество): число узлов
    // count(set): the number of nodes
    XPathValue functionCount(const FunctionCall& call)
    {
        XPathValue value = argument(call, 0);

        if (value.getType() != ValueType::NODE_SET)
            failEvaluation(i18n::format(i18n::Message::XPATH_COUNT_NOT_NODESET));
        return XPathValue::fromNumber(static_cast<double>(value.getNodes().size()));
    }

    // Строка идентификаторов для id(): из множества — строковые значения всех узлов
    // The identifier string for id(): from a set — the string values of all nodes
    static std::string idTokens(const XPathValue& value)
    {
        if (value.getType() != ValueType::NODE_SET)
            return value.toString();
        std::string tokens;
        for (const XPathItem& item : value.getNodes())
            tokens += stringValue(item) + " ";
        return tokens;
    }

    /*
    ** id('a b'): элементы с указанными ID в документе контекстного узла.
    ** ID-атрибуты определяет DTD (и xml:id) — как в Document::getElementById.
    **
    ** id('a b'): the elements with the given IDs in the document of the context node.
    ** ID attributes are defined by the DTD (and xml:id) — as in Document::getElementById.
    */
    XPathValue functionId(const FunctionCall& call)
    {
        const Node* node = call.context.item.node;
        std::vector<XPathItem> found;

        if (call.context.item.attribute != nullptr)
            node = call.context.item.owner;
        const Document* document = nullptr;
        if (node != nullptr)
            document = node->getOwnerDocument();
        std::string tokens = idTokens(argument(call, 0));
        for (std::string_view token : utils::splitTokens(tokens))
        {
            const Element* element = nullptr;
            if (document != nullptr)
                element = document->getElementById(token);
            if (element != nullptr)
                found.push_back(XPathItem{element, nullptr, nullptr});
        }
        sortDocumentOrder(found);
        return XPathValue::fromNodeSet(std::move(found));
    }
}
