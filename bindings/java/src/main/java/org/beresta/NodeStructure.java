package org.beresta;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.NodeFunctions;

/**
 * Изменение структуры: инструкции обработки, удаление, копирование и перенос поддерева.
 * <p>Structure changes: processing instructions, removal, copying and moving of a subtree.
 */
public interface NodeStructure extends NodeBase
{
    /**
     * Инструкция обработки {@code <?target data?>}; data может быть null.
     * <p>A processing instruction {@code <?target data?>}; data may be null.
     */
    default XmlNode appendProcessingInstruction(String target, String data)
    {
        try (Arena arena = Arena.ofConfined())
        {
            Object pointer = NativeCalls.invoke(NodeFunctions.APPEND_PI, handle(),
                NativeMemory.text(arena, target), NativeMemory.text(arena, data));

            return NodeFactory.wrap(pointer, document());
        }
    }

    /**
     * Удаление узла с потомками. После вызова этот объект и обёртки
     * потомков недействительны — обращаться к ним нельзя.
     *
     * <p>Removes the node with its descendants. After the call this object and the
     * wrappers of its descendants are invalid — they must not be used.
     */
    default void remove()
    {
        NativeCalls.check(NativeCalls.invoke(NodeFunctions.REMOVE, handle()));
    }

    /**
     * Глубокая копия в конец детей parent (можно в другой документ).
     * <p>A deep copy appended to the children of parent (another document is allowed).
     */
    default XmlNode copyTo(XmlNode parent)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(NodeFunctions.COPY_TO, handle(), parent.handle(), out));
            return NodeFactory.wrap(NativeMemory.readPointer(out), parent.document());
        }
    }

    /**
     * Перенос (без копирования) в конец детей parent, в том числе в другой
     * документ. Этот объект остаётся тем же узлом; после переноса его документ —
     * parent.document(), и закрытие прежнего документа узел не затрагивает.
     * Обёртки потомков, полученные раньше, тоже переходят к новому документу.
     * Корень документа и сам документ переносить нельзя (BerestaException
     * HIERARCHY_VIOLATION / INVALID_ARGUMENT).
     *
     * <p>Moves the node (without copying) to the end of the children of parent,
     * including into another document. This object stays the same node; after the move
     * its document is parent.document(), and closing the former document does not affect
     * the node. The wrappers of descendants obtained earlier move to the new document too.
     * The document root and the document itself cannot be moved (BerestaException
     * HIERARCHY_VIOLATION / INVALID_ARGUMENT).
     */
    default void moveTo(XmlNode parent)
    {
        XmlDocument source = document();
        XmlDocument target = parent.document();
        MemorySegment pointer = handle();

        NativeCalls.check(NativeCalls.invoke(NodeFunctions.MOVE_TO, pointer, parent.handle()));
        if (source != target)
            source.registry.transfer((XmlNode) this, pointer, target);
    }
}
