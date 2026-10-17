#include "serializer/xml_serializer.hpp"

namespace beresta
{
    /*
    ** Без prettyPrint — обычная монолитная сериализация узла (только
    ** декларация может быть подменена). С prettyPrint — обход с отступами.
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
    void XmlSerializer::writeNode(const Node& node, std::size_t depth, std::string& output) const
    {
        if (_options.prettyPrint && node.getType() == NodeType::ELEMENT)
            writePrettyElement(static_cast<const Element&>(node), depth, output);
        else
            node.serialize(output);
    }

    /*
    ** Элемент без детей или с текстом внутри выводится дословно: отступы
    ** внутри смешанного содержимого изменили бы данные. Элемент только
    ** с элементами/комментариями получает каждого ребёнка на новой строке.
    */
    void XmlSerializer::writePrettyElement(const Element& element, std::size_t depth,
        std::string& output) const
    {
        if (element.getChildCount() == 0 || hasTextualChild(element))
        {
            element.serialize(output);
            return;
        }
        element.serializeStartTag(output);
        output += '>';
        for (const Node* child = element.getFirstChild(); child != nullptr;
            child = child->getNextSibling())
        {
            output += _options.newline;
            writeIndent(depth + 1, output);
            writeNode(*child, depth + 1, output);
        }
        output += _options.newline;
        writeIndent(depth, output);
        output += "</" + element.getName() + ">";
    }

    // Отступ — повторение строки indent depth раз
    void XmlSerializer::writeIndent(std::size_t depth, std::string& output) const
    {
        for (std::size_t level = 0; level < depth; ++level)
            output += _options.indent;
    }

    // Узлы верхнего уровня — каждый на своей строке, в конце — перевод строки
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
