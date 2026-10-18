#include "bench/engines/beresta_engine.hpp"

#include "capi/beresta_c.h"

#include <vector>

namespace bench
{
    // Название и версия / The name and the version
    std::string BerestaEngine::name() const
    {
        return std::string("Берёста / Beresta ") + beresta_version();
    }

    // Разбор в новое дерево; старое уничтожается присваиванием / Parsing into a new tree; the old one is destroyed by assignment
    void BerestaEngine::parse(const std::string& xml)
    {
        _document = std::make_unique<beresta::Document>(beresta::XmlParser().parse(xml));
    }

    // Вклад узла в контрольную сумму: элемент + его атрибуты, текст и CDATA — по одному
    // The node contribution to the checksum: an element + its attributes, text and CDATA — one each
    static std::size_t weight(const beresta::Node& node)
    {
        beresta::NodeType type = node.getType();

        if (type == beresta::NodeType::ELEMENT)
            return 1 + static_cast<const beresta::Element&>(node).getAttributes().size();
        if (type == beresta::NodeType::TEXT || type == beresta::NodeType::CDATA_SECTION)
            return 1;
        return 0;
    }

    /*
    ** Обход всего дерева по явному стеку (как у остальных библиотек):
    ** порядок обхода на сумму не влияет, а глубина не ограничена стеком вызовов.
    **
    ** A walk over the whole tree with an explicit stack (as for the other
    ** libraries): the order does not affect the sum, and the depth is not
    ** limited by the call stack.
    */
    std::size_t BerestaEngine::traverse() const
    {
        std::vector<const beresta::Node*> stack{_document.get()};
        std::size_t sum = 0;

        while (!stack.empty())
        {
            const beresta::Node* node = stack.back();
            stack.pop_back();
            sum += weight(*node);
            beresta::NodeType type = node->getType();
            if (type != beresta::NodeType::ELEMENT && type != beresta::NodeType::DOCUMENT)
                continue;
            const auto* container = static_cast<const beresta::ContainerNode*>(node);
            for (const beresta::Node* child = container->getFirstChild(); child; child = child->getNextSibling())
                stack.push_back(child);
        }
        return sum;
    }

    // Освобождение дерева / Freeing the tree
    void BerestaEngine::release()
    {
        _document.reset();
    }
}
