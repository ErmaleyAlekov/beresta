#include "core/document.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

namespace beresta
{
    // Пустой документ без декларации, пролога и корня
    // An empty document without a declaration, a prolog and a root
    Document::Document() noexcept
        : ContainerNode(), _declaration()
    {
    }

    // Тип узла — документ
    // The node type is a document
    NodeType Document::getType() const noexcept
    {
        return NodeType::DOCUMENT;
    }

    // Глубокая копия всего документа вместе с декларацией
    // A deep copy of the whole document together with the declaration
    std::unique_ptr<Node> Document::clone() const
    {
        return std::make_unique<Document>(*this);
    }

    // Декларация (если есть), затем дети подряд — без разделителей
    // The declaration (if there is one), then the children in a row — without separators
    void Document::serialize(std::string& output) const
    {
        serializeOpen(output);
        serializeChildren(output);
        serializeClose(output);
    }

    /*
    ** Правила верхнего уровня XML:
    ** - Comment и PI — в любом месте и в любом количестве;
    ** - DocumentType — один и строго ДО корневого элемента;
    ** - Element — один и строго ПОСЛЕ DOCTYPE;
    ** - Text, CDATA и вложенный Document — запрещены.
    **
    ** The XML top-level rules:
    **  - Comment and PI — anywhere and in any number;
    **  - DocumentType — one and strictly BEFORE the root element;
    **  - Element — one and strictly AFTER the DOCTYPE;
    **  - Text, CDATA and a nested Document — forbidden.
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
                i18n::format(i18n::Message::DOCUMENT_ONE_ROOT));
        throw DomException(DomErrorCode::HIERARCHY_VIOLATION,
            i18n::format(i18n::Message::DOCUMENT_TOP_LEVEL_NODE));
    }
}
