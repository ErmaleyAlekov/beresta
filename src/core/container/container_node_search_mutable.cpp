#include "core/container_node.hpp"
#include "core/element.hpp"

namespace beresta
{
    /*
    ** Снимает константность с результатов поиска. Безопасно: поиск
    ** вызывается на неконстантном контейнере, значит и все найденные
    ** узлы его поддерева изначально неконстантны.
    */
    static std::vector<Element*> toMutable(const std::vector<const Element*>& found)
    {
        std::vector<Element*> result;

        result.reserve(found.size());
        for (const Element* element : found)
            result.push_back(const_cast<Element*>(element));
        return result;
    }

    // Глубокий поиск по имени (изменяемая версия)
    std::vector<Element*> ContainerNode::getElementsByTagName(std::string_view name)
    {
        const ContainerNode& self = *this;

        return toMutable(self.getElementsByTagName(name));
    }

    // Глубокий поиск по пространству имён (изменяемая версия)
    std::vector<Element*> ContainerNode::getElementsByTagNameNS(std::string_view namespaceUri,
        std::string_view localName)
    {
        const ContainerNode& self = *this;

        return toMutable(self.getElementsByTagNameNS(namespaceUri, localName));
    }

    // Попарное сравнение детей: одинаковое количество и равенство каждого
    bool ContainerNode::childrenEqual(const ContainerNode& other) const
    {
        if (_children.size() != other._children.size())
            return false;
        for (std::size_t index = 0; index < _children.size(); ++index)
        {
            if (!_children[index]->isEqualNode(*other._children[index]))
                return false;
        }
        return true;
    }
}
