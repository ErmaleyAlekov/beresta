'use strict';

const { native } = require('../internal/native.js');
const { REGISTRY } = require('../internal/keys.js');
const { wrap } = require('./wrapping.js');

/**
 * Примесь: удаление, копирование и перенос поддерева.
 * Mixin: removing, copying and moving a subtree.
 */
class NodeStructure
{
    /** Удаление узла с потомками. После вызова эта обёртка и обёртки потомков
     *  недействительны — обращаться к ним нельзя.
     *  Removes the node with its descendants. After the call this wrapper and the
     *  wrappers of its descendants are invalid — they must not be used. */
    remove()
    {
        native.node_remove(this.handle);
    }

    /** Глубокая копия в конец детей parent (можно в другой документ); возвращает копию.
     *  A deep copy appended to the children of parent (another document is allowed); returns the copy. */
    copyTo(parent)
    {
        return wrap(parent.document, native.node_copy_to(this.handle, parent.handle));
    }

    /** Перенос (без копирования) в конец детей parent, в том числе в другой документ.
     *  Эта обёртка остаётся тем же узлом; после переноса её документ (OWNER) —
     *  parent.document, и закрытие прежнего документа узел не затрагивает. Обёртки
     *  потомков, полученные раньше, тоже переходят к новому документу. Корень
     *  документа и сам документ переносить нельзя (HIERARCHY_VIOLATION / INVALID_ARGUMENT).
     *  Moves the node (without copying) to the end of the children of parent, including
     *  into another document. This wrapper stays the same node; after the move its document
     *  (OWNER) is parent.document, and closing the former document does not affect the node.
     *  The wrappers of descendants obtained earlier move to the new document too. The
     *  document root and the document itself cannot be moved (HIERARCHY_VIOLATION / INVALID_ARGUMENT). */
    moveTo(parent)
    {
        const source = this.document;
        const target = parent.document;
        const handle = this.handle;

        native.node_move_to(handle, parent.handle);
        if (source !== target)
            source[REGISTRY].transfer(this, handle, target);
    }
}

module.exports = { NodeStructure };
