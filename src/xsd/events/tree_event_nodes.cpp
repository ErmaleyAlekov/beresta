#include "xsd/internal/tree_event_source.hpp"
#include "core/character_node.hpp"
#include "core/processing_instruction.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Вход в узел. Элемент открывается; если детей нет — сразу и
    ** закрывается. Остальные узлы — одно событие. true — нужно спуститься
    ** к первому ребёнку.
    **
    ** Entering a node. An element is opened; if it has no children — it is closed
    ** right away. Other nodes are one event. true — descending to the first
    ** child is needed.
    */
    bool TreeEventSource::enter(const Node& node, ContentHandler& handler)
    {
        NodeType type = node.getType();

        _current = &node;
        if (type == NodeType::ELEMENT)
        {
            startElement(static_cast<const Element&>(node), handler);
            if (static_cast<const Element&>(node).getFirstChild() != nullptr)
                return true;
            endElement(static_cast<const Element&>(node), handler);
        }
        else if (type == NodeType::TEXT)
            handler.characters(static_cast<const CharacterNode&>(node).getValue());
        else if (type == NodeType::CDATA_SECTION)
            handler.cdata(static_cast<const CharacterNode&>(node).getValue());
        else if (type == NodeType::COMMENT)
            handler.comment(static_cast<const CharacterNode&>(node).getValue());
        else if (type == NodeType::PROCESSING_INSTRUCTION)
            handler.processingInstruction(static_cast<const ProcessingInstruction&>(node).getTarget(),
                static_cast<const ProcessingInstruction&>(node).getData());
        return false;
    }

    // Выход из узла: следующий сосед или подъём с закрытием предков (до корня включительно)
    // Leaving a node: the next sibling or climbing with closing the ancestors (up to and including the root)
    const Node* TreeEventSource::leave(const Node& node, const Node& root, ContentHandler& handler)
    {
        const Node* current = &node;

        while (current != &root)
        {
            if (current->getNextSibling() != nullptr)
                return current->getNextSibling();
            current = current->getParent();
            endElement(static_cast<const Element&>(*current), handler);
        }
        return nullptr;
    }

    // Атрибуты — представлениями строк дерева; пространства имён разрешит валидатор
    // Attributes — as views of the tree strings; the validator resolves the namespaces
    void TreeEventSource::startElement(const Element& element, ContentHandler& handler)
    {
        _attributes.clear();
        for (const Attribute& attribute : element.getAttributes())
            _attributes.push_back(SaxAttribute{attribute.name, attribute.value, std::string_view(),
                utils::splitQualifiedName(attribute.name).localName});
        handler.startElement(SaxElement{element.getName(), std::string_view(), element.getLocalName(), _attributes});
    }

    // Закрытие элемента; список атрибутов в endElement всегда пуст
    // Closing an element; the attribute list in endElement is always empty
    void TreeEventSource::endElement(const Element& element, ContentHandler& handler)
    {
        static const std::vector<SaxAttribute> NO_ATTRIBUTES;

        handler.endElement(SaxElement{element.getName(), std::string_view(), element.getLocalName(), NO_ATTRIBUTES});
    }
}
