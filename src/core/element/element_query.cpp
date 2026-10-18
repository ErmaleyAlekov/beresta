#include "core/element.hpp"

namespace beresta
{
    // Изменяемая версия переиспользует константную (DRY)
    // The mutable version reuses the const one (DRY)
    Element* Element::findChildElement(std::string_view name)
    {
        const Element& self = *this;

        return const_cast<Element*>(self.findChildElement(name));
    }

    /*
    ** Перебираем непосредственных детей; тип проверяется до приведения,
    ** поэтому static_cast безопасен и не требует RTTI.
    **
    ** Iterates over the direct children; the type is checked before the cast,
    ** so static_cast is safe and does not require RTTI.
    */
    const Element* Element::findChildElement(std::string_view name) const
    {
        for (std::size_t index = 0; index < getChildCount(); ++index)
        {
            const Node* child = getChild(index);
            if (child->getType() != NodeType::ELEMENT)
                continue;
            const Element* element = static_cast<const Element*>(child);
            if (element->getName() == name)
                return element;
        }
        return nullptr;
    }

    // Изменяемые указатели на все дочерние элементы с нужным именем
    // Mutable pointers to all child elements with the required name
    std::vector<Element*> Element::findChildElements(std::string_view name)
    {
        std::vector<Element*> found;

        for (std::size_t index = 0; index < getChildCount(); ++index)
        {
            Node* child = getChild(index);
            if (child->getType() != NodeType::ELEMENT)
                continue;
            Element* element = static_cast<Element*>(child);
            if (element->getName() == name)
                found.push_back(element);
        }
        return found;
    }

    // Константные указатели на все дочерние элементы с нужным именем
    // Const pointers to all child elements with the required name
    std::vector<const Element*> Element::findChildElements(std::string_view name) const
    {
        std::vector<const Element*> found;

        for (std::size_t index = 0; index < getChildCount(); ++index)
        {
            const Node* child = getChild(index);
            if (child->getType() != NodeType::ELEMENT)
                continue;
            const Element* element = static_cast<const Element*>(child);
            if (element->getName() == name)
                found.push_back(element);
        }
        return found;
    }
}
