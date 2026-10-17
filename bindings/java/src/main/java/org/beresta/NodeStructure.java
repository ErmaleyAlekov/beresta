package org.beresta;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.NodeFunctions;

/** Изменение структуры: инструкции обработки, удаление и копирование поддерева. */
public interface NodeStructure extends NodeBase
{
    /** Инструкция обработки {@code <?target data?>}; data может быть null. */
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
     */
    default void remove()
    {
        NativeCalls.check(NativeCalls.invoke(NodeFunctions.REMOVE, handle()));
    }

    /** Глубокая копия в конец детей parent (можно в другой документ). */
    default XmlNode copyTo(XmlNode parent)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(NodeFunctions.COPY_TO, handle(), parent.handle(), out));
            return NodeFactory.wrap(NativeMemory.readPointer(out), parent.document());
        }
    }
}
