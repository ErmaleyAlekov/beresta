#include "capi/internal/capi_support.hpp"

/*
** Навигация по дереву. Все функции устроены одинаково: найти узел
** за дескриптором, взять соседа / родителя / ребёнка и вернуть его как
** Node* -> void*. Отсутствие соседа — NULL без ошибки (код 0).
** У листовых узлов (текст, комментарий) детей нет — тоже NULL.
**
** Tree navigation. All functions work the same way: find the node behind
** the handle, take the sibling / parent / child and return it as
** Node* -> void*. A missing sibling is NULL without an error (code 0).
** Leaf nodes (text, comment) have no children — NULL too.
*/

extern "C" BERESTA_API beresta_node beresta_node_parent(beresta_node node)
{
    BERESTA_CAPI_TRY
        beresta::Node* parent = beresta::capi::toNode(node).getParent();

        return parent;
    BERESTA_CAPI_CATCH(nullptr)
}

extern "C" BERESTA_API beresta_node beresta_node_first_child(beresta_node node)
{
    BERESTA_CAPI_TRY
        beresta::ContainerNode* container = dynamic_cast<beresta::ContainerNode*>(&beresta::capi::toNode(node));

        if (container == nullptr)
            return nullptr;
        return container->getFirstChild();
    BERESTA_CAPI_CATCH(nullptr)
}

extern "C" BERESTA_API beresta_node beresta_node_last_child(beresta_node node)
{
    BERESTA_CAPI_TRY
        beresta::ContainerNode* container = dynamic_cast<beresta::ContainerNode*>(&beresta::capi::toNode(node));

        if (container == nullptr)
            return nullptr;
        return container->getLastChild();
    BERESTA_CAPI_CATCH(nullptr)
}

// Соседи за O(1): узел хранит свой индекс у родителя
// Siblings in O(1): a node keeps its index in the parent
extern "C" BERESTA_API beresta_node beresta_node_next_sibling(beresta_node node)
{
    BERESTA_CAPI_TRY
        return beresta::capi::toNode(node).getNextSibling();
    BERESTA_CAPI_CATCH(nullptr)
}

extern "C" BERESTA_API beresta_node beresta_node_previous_sibling(beresta_node node)
{
    BERESTA_CAPI_TRY
        return beresta::capi::toNode(node).getPreviousSibling();
    BERESTA_CAPI_CATCH(nullptr)
}
