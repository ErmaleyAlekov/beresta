#include "core/container_node.hpp"
#include "core/element.hpp"

namespace beresta
{
    /*
    ** Обход поддерева в порядке документа (pre-order) БЕЗ рекурсии:
    ** явный стек хранит пары «контейнер — индекс следующего ребёнка».
    ** Глубокие деревья не переполнят системный стек вызовов.
    ** match — ссылка на функцию-предикат, отбирающую нужные элементы.
    */
    static void collectElements(const ContainerNode& root,
        bool (*match)(const Element&, std::string_view, std::string_view),
        std::string_view first, std::string_view second,
        std::vector<const Element*>& found)
    {
        std::vector<std::pair<const ContainerNode*, std::size_t>> stack;

        stack.emplace_back(&root, 0);
        while (!stack.empty())
        {
            std::pair<const ContainerNode*, std::size_t>& top = stack.back();
            if (top.second >= top.first->getChildCount())
            {
                stack.pop_back();
                continue;
            }
            const Node* child = top.first->getChild(top.second++);
            if (child->getType() != NodeType::ELEMENT)
                continue;
            const Element* element = static_cast<const Element*>(child);
            if (match(*element, first, second))
                found.push_back(element);
            stack.emplace_back(element, 0);
        }
    }

    // Предикат «имя тега совпадает или запрошены все элементы»
    static bool matchesTagName(const Element& element, std::string_view name,
        std::string_view unused)
    {
        (void)unused;
        return name == "*" || element.getName() == name;
    }

    // Предикат «пространство имён и локальное имя совпадают ("*" — любое)»
    static bool matchesNamespace(const Element& element, std::string_view namespaceUri,
        std::string_view localName)
    {
        if (localName != "*" && element.getLocalName() != localName)
            return false;
        if (namespaceUri == "*")
            return true;
        return element.getNamespaceURI().value_or("") == namespaceUri;
    }

    // Глубокий поиск по имени тега (константная версия)
    std::vector<const Element*> ContainerNode::getElementsByTagName(std::string_view name) const
    {
        std::vector<const Element*> found;

        collectElements(*this, matchesTagName, name, "", found);
        return found;
    }

    // Глубокий поиск по пространству имён и локальному имени (константная версия)
    std::vector<const Element*> ContainerNode::getElementsByTagNameNS(
        std::string_view namespaceUri, std::string_view localName) const
    {
        std::vector<const Element*> found;

        collectElements(*this, matchesNamespace, namespaceUri, localName, found);
        return found;
    }
}
