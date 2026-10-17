#include "core/document.hpp"
#include "exception/dom_exception.hpp"

namespace beresta
{
    // Пустой документ без декларации, пролога и корня
    Document::Document() noexcept
        : ContainerNode(), _declaration()
    {
    }

    // Тип узла — документ
    NodeType Document::getType() const noexcept
    {
        return NodeType::DOCUMENT;
    }

    // Глубокая копия всего документа вместе с декларацией
    std::unique_ptr<Node> Document::clone() const
    {
        return std::make_unique<Document>(*this);
    }

    // Декларация (если есть), затем дети подряд — без разделителей
    void Document::serialize(std::string& output) const
    {
        if (_declaration.has_value())
            serializeXmlDeclaration(_declaration.value(), output);
        serializeChildren(output);
    }

    /*
    ** Правила верхнего уровня XML:
    ** - Comment и PI — в любом месте и в любом количестве;
    ** - DocumentType — один и строго ДО корневого элемента;
    ** - Element — один и строго ПОСЛЕ DOCTYPE;
    ** - Text, CDATA и вложенный Document — запрещены.
    */
    void Document::validateChild(const Node& child, std::size_t index) const
    {
        NodeType type = child.getType();
        std::size_t root = findFirstOfType(NodeType::ELEMENT);
        std::size_t doctype = findFirstOfType(NodeType::DOCUMENT_TYPE);

        if (type == NodeType::COMMENT || type == NodeType::PROCESSING_INSTRUCTION)
            return;
        if (type == NodeType::DOCUMENT_TYPE && doctype == npos && (root == npos || index <= root))
            return;
        if (type == NodeType::ELEMENT && root == npos && (doctype == npos || index > doctype))
            return;
        if (type == NodeType::ELEMENT || type == NodeType::DOCUMENT_TYPE)
            throw DomException(DomErrorCode::HIERARCHY_VIOLATION,
                "документ может содержать один корневой элемент и один DOCTYPE перед ним");
        throw DomException(DomErrorCode::HIERARCHY_VIOLATION,
            "на верхнем уровне документа допустимы только элемент, DOCTYPE, комментарии и PI");
    }
}
