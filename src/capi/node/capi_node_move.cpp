#include "capi/internal/capi_support.hpp"
#include "exception/dom_exception.hpp"
#include "i18n/i18n.hpp"

namespace
{
    /*
    ** Проверки ДО отсоединения узла: после removeChild ошибку уже
    ** пришлось бы исправлять возвратом узла на место, а здесь дерево
    ** ещё не тронуто. Запрещено:
    **   - переносить документ (у него нет родителя) — неверный аргумент;
    **   - переносить корневой элемент: документ остался бы без корня
    **     (корень переносят вместе с документом — beresta_document_clone —
    **     или копируют через beresta_node_copy_to);
    **   - переносить узел в самого себя или в своего потомка — цикл.
    ** Допустимость типа ребёнка в новом родителе проверит само ядро.
    **
    ** Checks BEFORE the node is detached: after removeChild an error would
    ** have to be repaired by putting the node back, while here the tree is
    ** still untouched. It is forbidden:
    **   - to move a document (it has no parent) — an invalid argument;
    **   - to move the root element: the document would be left without a root
    **     (the root travels with its document — beresta_document_clone —
    **     or is copied through beresta_node_copy_to);
    **   - to move a node into itself or into its descendant — a cycle.
    ** The core itself checks whether the child type is allowed in the new parent.
    */
    void checkMovable(const beresta::Node& node, const beresta::ContainerNode& parent)
    {
        const beresta::ContainerNode* current = node.getParent();

        if (current == nullptr)
            throw beresta::capi::InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::MOVE_DOCUMENT));
        if (node.getType() == beresta::NodeType::ELEMENT && current->getType() == beresta::NodeType::DOCUMENT)
            throw beresta::DomException(beresta::DomErrorCode::HIERARCHY_VIOLATION,
                beresta::i18n::format(beresta::i18n::Message::MOVE_DOCUMENT_ROOT));
        if (&parent == &node || parent.isDescendantOf(node))
            throw beresta::DomException(beresta::DomErrorCode::HIERARCHY_VIOLATION,
                beresta::i18n::format(beresta::i18n::Message::MOVE_ANCESTOR_INTO_DESCENDANT));
    }

    /*
    ** Сам перенос: старый родитель отдаёт владение (unique_ptr), новый
    ** принимает его. Объект узла при этом не копируется и не пересоздаётся,
    ** поэтому дескрипторы узла и всех его потомков остаются прежними —
    ** просто теперь ими владеет новый документ, и освобождение старого
    ** документа их не затрагивает.
    **
    ** Если новый родитель отверг узел (например, текст на верхнем уровне
    ** документа), appendChild оставляет владение у нас (он принимает
    ** rvalue-ссылку) — и узел возвращается на прежнее место. Вставка по
    ** старому индексу в того же родителя заведомо допустима, а память
    ** вектора после erase не освобождалась, так что повторного выделения
    ** нет. Итог — строгая гарантия: при ошибке дерево не меняется.
    **
    ** The move itself: the old parent gives up the ownership (unique_ptr), the new one
    ** takes it. The node object is neither copied nor recreated, so the handles of
    ** the node and all its descendants stay the same — they are simply owned by the
    ** new document now, and freeing the old document does not touch them.
    **
    ** If the new parent rejected the node (for example, text at the top level of
    ** a document), appendChild leaves the ownership with us (it takes an rvalue
    ** reference) — and the node goes back to its old place. Inserting at the old
    ** index into the same parent is certainly allowed, and the vector memory was not
    ** released by erase, so there is no reallocation. The result is the strong
    ** guarantee: on error the tree does not change.
    */
    void moveNode(beresta::Node& node, beresta::ContainerNode& parent)
    {
        beresta::ContainerNode& source = *node.getParent();
        std::size_t index = source.indexOf(&node);
        std::unique_ptr<beresta::Node> detached = source.removeChild(index);

        try
        {
            parent.appendChild(std::move(detached));
        }
        catch (...)
        {
            source.insertChild(index, std::move(detached));
            throw;
        }
    }
}

/*
** Перенос узла (без копирования) в конец детей parent — в том числе
** в другой документ. Это то же, что removeChild + appendChild в C++.
**
** Moves a node (without copying) to the end of the children of parent —
** including into another document. This is the same as removeChild + appendChild in C++.
*/
extern "C" BERESTA_API int beresta_node_move_to(beresta_node node, beresta_node parent)
{
    BERESTA_CAPI_TRY
        beresta::Node& moved = beresta::capi::toNode(node);
        beresta::ContainerNode& target = beresta::capi::toContainer(parent);

        checkMovable(moved, target);
        moveNode(moved, target);
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}
