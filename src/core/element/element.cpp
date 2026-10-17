#include "core/element.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Имя проходит проверку в setName(), поэтому элемент не бывает «битым»
    Element::Element(std::string name)
        : ContainerNode()
    {
        setName(std::move(name));
    }

    // Тип узла — элемент
    NodeType Element::getType() const noexcept
    {
        return NodeType::ELEMENT;
    }

    // Глубокая копия через конструктор копирования (дети клонируются рекурсивно)
    std::unique_ptr<Node> Element::clone() const
    {
        return std::make_unique<Element>(*this);
    }

    /*
    ** Монолитная сериализация без лишних пробельных символов:
    ** - элемент без детей:  <name attr="v"/>
    ** - элемент с детьми:   <name attr="v">дети</name>
    ** Никаких отступов и переводов строк между тегами не добавляется.
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
        output += "</";
        output += _name;
        output += '>';
    }

    // Документ и DOCTYPE существуют только на верхнем уровне
    void Element::validateChild(const Node& child, std::size_t index) const
    {
        NodeType type = child.getType();

        (void)index;
        if (type == NodeType::DOCUMENT || type == NodeType::DOCUMENT_TYPE)
            throw DomException(DomErrorCode::HIERARCHY_VIOLATION,
                "документ и DOCTYPE не могут быть дочерними узлами элемента <" + _name + ">");
    }
}
