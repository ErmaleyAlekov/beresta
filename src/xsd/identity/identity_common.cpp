#include "xsd/internal/identity_common.hpp"

namespace beresta::xsd::detail
{
    // Значения полей для сообщения: "A1, 2026"
    // The field values for a message: "A1, 2026"
    std::string joinKeyValues(const std::vector<std::string>& values)
    {
        std::string result;

        for (const std::string& value : values)
        {
            if (!result.empty())
                result += ", ";
            result += value;
        }
        return result;
    }

    // xs:keyref повторы не проверяет, поэтому меток две
    // xs:keyref does not check repetitions, so there are two labels
    std::string uniquenessLabel(const IdentityConstraint& constraint)
    {
        if (constraint.kind == IdentityKind::KEY)
            return "xs:key";
        return "xs:unique";
    }

    // Сегмент пути ребёнка с номером среди одноимённых соседей, если их несколько
    // A child path segment with the number among same-named siblings, if there are several
    std::string childPath(const std::string& parentPath, const Element& child)
    {
        std::size_t index = 0;
        std::size_t total = 0;

        for (const Node* node = child.getParent()->getFirstChild(); node != nullptr; node = node->getNextSibling())
        {
            if (node->getType() != NodeType::ELEMENT || static_cast<const Element*>(node)->getName() != child.getName())
                continue;
            ++total;
            if (node == &child)
                index = total;
        }
        std::string path = parentPath + "/" + child.getName();
        if (total > 1)
            path += "[" + std::to_string(index) + "]";
        return path;
    }

    // Путь от корня: предки собираются снизу вверх, путь строится сверху вниз
    // The path from the root: the ancestors are collected bottom-up, the path is built top-down
    std::string pathOf(const Element& element)
    {
        std::vector<const Element*> chain;

        for (const Node* node = &element; node != nullptr; node = node->getParent())
        {
            if (node->getType() == NodeType::ELEMENT)
                chain.push_back(static_cast<const Element*>(node));
        }
        std::string path = "/" + chain.back()->getName();
        for (std::size_t index = chain.size() - 1; index > 0; --index)
            path = childPath(path, *chain[index - 1]);
        return path;
    }

    /*
    ** Прямой порядок без рекурсии и без стека: первый ребёнок-элемент,
    ** иначе следующий сосед-элемент — свой или ближайшего предка (не
    ** выходя за root). Глубина документа не ограничивает обход.
    **
    ** Document order without recursion and without a stack: the first child element,
    ** otherwise the next sibling element — its own or of the nearest ancestor (without
    ** leaving root). The document depth does not limit the traversal.
    */
    const Element* nextElement(const Element& current, const Element& root) noexcept
    {
        for (const Node* child = current.getFirstChild(); child != nullptr; child = child->getNextSibling())
        {
            if (child->getType() == NodeType::ELEMENT)
                return static_cast<const Element*>(child);
        }
        for (const Node* node = &current; node != nullptr && node != &root; node = node->getParent())
        {
            for (const Node* next = node->getNextSibling(); next != nullptr; next = next->getNextSibling())
            {
                if (next->getType() == NodeType::ELEMENT)
                    return static_cast<const Element*>(next);
            }
        }
        return nullptr;
    }
}
