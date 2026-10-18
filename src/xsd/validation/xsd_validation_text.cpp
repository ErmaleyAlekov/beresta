#include "xsd/internal/validation_run.hpp"
#include "core/character_node.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::xsd::detail
{
    // Узел с текстом: обычный текст или секция CDATA / A node with text: plain text or a CDATA section
    static bool isTextNode(const Node& node) noexcept
    {
        return node.getType() == NodeType::TEXT || node.getType() == NodeType::CDATA_SECTION;
    }

    /*
    ** Текст элемента простого типа. Обычно это ровно один текстовый узел, и
    ** тогда его значение отдаётся представлением без копии (описание товара
    ** может быть длинным). Иначе (несколько узлов, комментарии между ними)
    ** текст собирается в buffer, который вызывающий держит живым.
    **
    ** The text of a simple-typed element. Usually it is exactly one text node, and
    ** then its value is returned as a view without a copy (a product description
    ** may be long). Otherwise (several nodes, comments between them)
    ** the text is collected into buffer, which the caller keeps alive.
    */
    std::string_view textOf(const Element& element, std::string& buffer)
    {
        const Node* first = element.getFirstChild();

        if (first == nullptr)
            return std::string_view();
        if (first->getNextSibling() == nullptr && isTextNode(*first))
            return static_cast<const CharacterNode*>(first)->getValue();
        buffer = element.getTextContent();
        return buffer;
    }

    // Значимый текст — непробельный TEXT или любая CDATA (значение читается без копии)
    // Significant text is a non-whitespace TEXT or any CDATA (the value is read without a copy)
    bool hasSignificantText(const Element& element)
    {
        for (const Node* child = element.getFirstChild(); child != nullptr; child = child->getNextSibling())
        {
            if (child->getType() == NodeType::CDATA_SECTION)
                return true;
            if (child->getType() == NodeType::TEXT
                && !utils::isWhitespaceOnly(static_cast<const CharacterNode*>(child)->getValue()))
                return true;
        }
        return false;
    }
}
