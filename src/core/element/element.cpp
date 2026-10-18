#include "core/element.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Имя проходит проверку в setName(), поэтому элемент не бывает «битым»
    // The name is checked in setName(), so an element is never "broken"
    Element::Element(std::string name)
        : ContainerNode()
    {
        setName(std::move(name));
    }

    // Тип узла — элемент
    // The node type is an element
    NodeType Element::getType() const noexcept
    {
        return NodeType::ELEMENT;
    }

    // Глубокая копия через конструктор копирования (дети клонируются без рекурсии)
    // A deep copy through the copy constructor (the children are cloned without recursion)
    std::unique_ptr<Node> Element::clone() const
    {
        return std::make_unique<Element>(*this);
    }

    /*
    ** Монолитная сериализация без лишних пробельных символов:
    ** - элемент без детей:  <name attr="v"/>
    ** - элемент с детьми:   <name attr="v">дети</name>
    ** Никаких отступов и переводов строк между тегами не добавляется.
    **
    ** Monolithic serialization without extra whitespace:
    **  - an element without children:  <name attr="v"/>
    **  - an element with children:     <name attr="v">children</name>
    ** No indentation or line breaks are added between tags.
    */
    void Element::serialize(std::string& output) const
    {
        serializeStartTag(output);
        if (getChildCount() == 0)
        {
            output += "/>";
            return;
        }
        output += '>';
        serializeChildren(output);
        serializeClose(output);
    }

    // Документ и DOCTYPE существуют только на верхнем уровне
    // A document and a DOCTYPE exist only at the top level
    void Element::validateChild(const Node& child, std::size_t index) const
    {
        NodeType type = child.getType();

        (void)index;
        if (type == NodeType::DOCUMENT || type == NodeType::DOCUMENT_TYPE)
            throw DomException(DomErrorCode::HIERARCHY_VIOLATION,
                i18n::format(i18n::Message::ELEMENT_CHILD_DOCUMENT, {_name}));
    }
}
