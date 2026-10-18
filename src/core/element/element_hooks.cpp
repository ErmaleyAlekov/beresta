#include "core/element.hpp"

namespace beresta
{
    /*
    ** Крючки, через которые ContainerNode обходит дерево без рекурсии.
    ** Каждый описывает элемент «сам по себе» — без детей: обход детей
    ** целиком лежит на ContainerNode (паттерн Template Method).
    **
    ** The hooks through which ContainerNode traverses the tree without recursion.
    ** Each describes the element "by itself" — without children: the traversal of the children
    ** lies entirely on ContainerNode (the Template Method pattern).
    */

    // Пустая копия: имя, атрибуты и позиция в исходнике, без детей
    // An empty copy: the name, the attributes and the source position, without children
    std::unique_ptr<ContainerNode> Element::cloneEmpty() const
    {
        return cloneShallow();
    }

    // Атрибуты равны как множество пар «имя — значение»: порядок не важен
    // Attributes are equal as a set of "name — value" pairs: the order does not matter
    static bool attributesEqual(const std::vector<Attribute>& left, const Element& right)
    {
        if (left.size() != right.getAttributes().size())
            return false;
        for (const Attribute& attribute : left)
        {
            std::optional<std::string> value = right.getAttribute(attribute.name);
            if (!value.has_value() || value.value() != attribute.value)
                return false;
        }
        return true;
    }

    // Собственные свойства элемента: имя и атрибуты (дети сравниваются отдельно)
    // The element's own properties: the name and the attributes (children are compared separately)
    bool Element::equalsShallow(const ContainerNode& other) const
    {
        const Element& element = static_cast<const Element&>(other);

        return element._name == _name && attributesEqual(_attributes, element);
    }

    // Начальный тег непустого элемента: <имя атрибуты>
    // The start tag of a non-empty element: <name attributes>
    void Element::serializeOpen(std::string& output) const
    {
        serializeStartTag(output);
        output += '>';
    }

    // Конечный тег: </имя>
    // The end tag: </name>
    void Element::serializeClose(std::string& output) const
    {
        output += "</";
        output += _name;
        output += '>';
    }
}
