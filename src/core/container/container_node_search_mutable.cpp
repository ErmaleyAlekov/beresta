#include "core/container_node.hpp"
#include "core/element.hpp"

namespace beresta
{
    /*
    ** Снимает константность с результатов поиска. Безопасно: поиск
    ** вызывается на неконстантном контейнере, значит и все найденные
    ** узлы его поддерева изначально неконстантны.
    **
    ** Removes the constness from the search results. It is safe: the search
    ** is called on a non-const container, so all found nodes of its subtree
    ** are originally non-const too.
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
    // A deep search by name (the mutable version)
    std::vector<Element*> ContainerNode::getElementsByTagName(std::string_view name)
    {
        const ContainerNode& self = *this;

        return toMutable(self.getElementsByTagName(name));
    }

    // Глубокий поиск по пространству имён (изменяемая версия)
    // A deep search by namespace (the mutable version)
    std::vector<Element*> ContainerNode::getElementsByTagNameNS(std::string_view namespaceUri,
        std::string_view localName)
    {
        const ContainerNode& self = *this;

        return toMutable(self.getElementsByTagNameNS(namespaceUri, localName));
    }
}
