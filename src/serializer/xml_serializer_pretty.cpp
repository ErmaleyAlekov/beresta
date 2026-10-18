#include "serializer/xml_serializer.hpp"

namespace beresta
{
    /*
    ** Без prettyPrint — обычная монолитная сериализация узла (только
    ** декларация может быть подменена). С prettyPrint — обход с отступами.
    **
    ** Without prettyPrint — the ordinary monolithic serialization of the node (only
    ** the declaration may be substituted). With prettyPrint — a traversal with indentation.
    */
    std::string XmlSerializer::serializeUtf8(const Node& node) const
    {
        std::string output;

        writeDeclaration(node, output);
        if (node.getType() != NodeType::DOCUMENT)
        {
            writeNode(node, 0, output);
            return output;
        }
        const Document& document = static_cast<const Document&>(node);
        if (_options.prettyPrint)
        {
            writePrettyDocument(document, output);
            return output;
        }
        for (const Node* child = document.getFirstChild(); child != nullptr;
            child = child->getNextSibling())
        {
            child->serialize(output);
        }
        return output;
    }

    // Элементы — с отступами, остальные узлы — как есть
    // Elements are indented, other nodes are written as is
    void XmlSerializer::writeNode(const Node& node, std::size_t depth, std::string& output) const
    {
        if (_options.prettyPrint && node.getType() == NodeType::ELEMENT)
            writePrettyElement(static_cast<const Element&>(node), depth, output);
        else
            node.serialize(output);
    }

    /*
    ** Элемент с отступами — без рекурсии. Открытые элементы лежат в явном
    ** стеке вместе со «следующим ребёнком»: каждый шаг выводит одного
    ** ребёнка на новой строке, а когда дети кончились — конечный тег.
    ** Глубина документа ограничена только памятью, а не стеком вызовов.
    **
    ** An indented element — without recursion. Open elements lie on an explicit
    ** stack together with the "next child": each step writes one
    ** child on a new line, and when the children are over — the end tag.
    ** The document depth is limited only by memory, not by the call stack.
    */
    void XmlSerializer::writePrettyElement(const Element& element, std::size_t depth,
        std::string& output) const
    {
        std::vector<PrettyFrame> stack;

        openPrettyElement(element, depth, output, stack);
        while (!stack.empty())
        {
            PrettyFrame& top = stack.back();
            if (top.next == nullptr)
            {
                output += _options.newline;
                writeIndent(top.depth, output);
                output += "</" + top.element->getName() + ">";
                stack.pop_back();
                continue;
            }
            const Node& child = *top.next;
            std::size_t childDepth = top.depth + 1;
            top.next = child.getNextSibling();
            output += _options.newline;
            writeIndent(childDepth, output);
            if (child.getType() == NodeType::ELEMENT)
                openPrettyElement(static_cast<const Element&>(child), childDepth, output, stack);
            else
                child.serialize(output);
        }
    }

    /*
    ** Начало элемента. Пустой элемент и элемент со смешанным содержимым
    ** выводятся целиком и дословно (отступы внутри текста изменили бы
    ** данные). Иначе — начальный тег и кадр в стеке для его детей.
    **
    ** The start of an element. An empty element and an element with mixed content
    ** are written whole and verbatim (indentation inside the text would change
    ** the data). Otherwise — the start tag and a frame on the stack for its children.
    */
    void XmlSerializer::openPrettyElement(const Element& element, std::size_t depth,
        std::string& output, std::vector<PrettyFrame>& stack) const
    {
        if (element.getChildCount() == 0 || hasTextualChild(element))
        {
            element.serialize(output);
            return;
        }
        element.serializeStartTag(output);
        output += '>';
        stack.push_back(PrettyFrame{&element, element.getFirstChild(), depth});
    }

    // Узлы верхнего уровня — каждый на своей строке, в конце — перевод строки
    // Top-level nodes — each on its own line, a line break at the end
    void XmlSerializer::writePrettyDocument(const Document& document, std::string& output) const
    {
        for (const Node* child = document.getFirstChild(); child != nullptr;
            child = child->getNextSibling())
        {
            if (!output.empty())
                output += _options.newline;
            writeNode(*child, 0, output);
        }
        output += _options.newline;
    }
}
