#include "capi/internal/capi_support.hpp"

/*
** Документ-владелец. У самого документа владелец — он сам: так обёрткам
** не нужно отдельно различать «узел» и «документ» при поиске корня.
*/
extern "C" BERESTA_API beresta_document beresta_node_owner_document(beresta_node node)
{
    BERESTA_CAPI_TRY
        beresta::Node& self = beresta::capi::toNode(node);
        beresta::Node* owner = self.getOwnerDocument();

        if (owner == nullptr && self.getType() == beresta::NodeType::DOCUMENT)
            owner = &self;
        return owner;
    BERESTA_CAPI_CATCH(nullptr)
}

// Число детей; у листовых узлов 0, при ошибке -1
extern "C" BERESTA_API int64_t beresta_node_child_count(beresta_node node)
{
    BERESTA_CAPI_TRY
        const beresta::ContainerNode* container =
            dynamic_cast<const beresta::ContainerNode*>(&beresta::capi::toNode(node));

        if (container == nullptr)
            return 0;
        return static_cast<int64_t>(container->getChildCount());
    BERESTA_CAPI_CATCH(-1)
}

// Ребёнок по индексу: индекс проверяется до обращения к ядру
extern "C" BERESTA_API beresta_node beresta_node_child(beresta_node node, int64_t index)
{
    BERESTA_CAPI_TRY
        beresta::ContainerNode& container = beresta::capi::toContainer(node);
        std::size_t position = beresta::capi::checkedIndex(index, container.getChildCount());

        return container.getChild(position);
    BERESTA_CAPI_CATCH(nullptr)
}
