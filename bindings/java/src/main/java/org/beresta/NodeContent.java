package org.beresta;

import java.lang.foreign.Arena;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeStrings;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.NodeFunctions;

/** Тип, имя и текст узла. */
public interface NodeContent extends NodeBase
{
    default NodeType type()
    {
        int code = (int) NativeCalls.invoke(NodeFunctions.TYPE, handle());

        if (code < 0)
            throw NativeCalls.lastError();
        return NodeType.fromCode(code);
    }

    /** Имя по соглашению DOM: тег, цель PI, "#text", "#comment"... */
    default String name()
    {
        return NativeStrings.take(NativeCalls.invoke(NodeFunctions.NAME, handle()));
    }

    /** Текстовое содержимое (у элемента — весь текст потомков). */
    default String text()
    {
        return NativeStrings.take(NativeCalls.invoke(NodeFunctions.TEXT, handle()));
    }

    /** У элемента все дети заменяются одним текстовым узлом. */
    default void setText(String value)
    {
        try (Arena arena = Arena.ofConfined())
        {
            NativeCalls.check(NativeCalls.invoke(NodeFunctions.SET_TEXT, handle(), NativeMemory.text(arena, value)));
        }
    }
}
