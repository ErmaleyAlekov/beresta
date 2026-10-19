package org.beresta;

import java.lang.foreign.Arena;

import org.beresta.internal.NodeCalls;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeStrings;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.NodeFunctions;

/** Тип, имя и текст узла. / Node type, name and text. */
public interface NodeContent extends NodeBase
{
    default NodeType type()
    {
        int code = NodeCalls.type(handle());

        if (code < 0)
            throw NativeCalls.lastError();
        return NodeType.fromCode(code);
    }

    /**
     * Имя по соглашению DOM: тег, цель PI, "#text", "#comment"...
     * <p>The name by the DOM convention: a tag, a PI target, "#text", "#comment"...
     */
    default String name()
    {
        return NativeStrings.take(NativeCalls.invoke(NodeFunctions.NAME, handle()));
    }

    /**
     * Текстовое содержимое (у элемента — весь текст потомков).
     * <p>The text content (for an element — all text of its descendants).
     */
    default String text()
    {
        return NativeStrings.take(NativeCalls.invoke(NodeFunctions.TEXT, handle()));
    }

    /**
     * У элемента все дети заменяются одним текстовым узлом.
     * <p>For an element all children are replaced with one text node.
     */
    default void setText(String value)
    {
        try (Arena arena = Arena.ofConfined())
        {
            NativeCalls.check(NativeCalls.invoke(NodeFunctions.SET_TEXT, handle(), NativeMemory.text(arena, value)));
        }
    }
}
