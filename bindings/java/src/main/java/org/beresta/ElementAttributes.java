package org.beresta;

import java.lang.foreign.Arena;

import org.beresta.internal.ElementFunctions;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeStrings;
import org.beresta.internal.NativeMemory;

/** Чтение и запись атрибутов элемента по имени. */
public interface ElementAttributes extends NodeBase
{
    /** Значение атрибута или null, если атрибута нет. */
    default String getAttribute(String name)
    {
        try (Arena arena = Arena.ofConfined())
        {
            return NativeStrings.take(NativeCalls.invoke(ElementFunctions.GET_ATTRIBUTE, handle(),
                NativeMemory.text(arena, name)));
        }
    }

    /** Установка или замена значения; недопустимое имя — BerestaException(INVALID_NAME). */
    default void setAttribute(String name, String value)
    {
        try (Arena arena = Arena.ofConfined())
        {
            NativeCalls.check(NativeCalls.invoke(ElementFunctions.SET_ATTRIBUTE, handle(),
                NativeMemory.text(arena, name), NativeMemory.text(arena, value)));
        }
    }

    default boolean hasAttribute(String name)
    {
        try (Arena arena = Arena.ofConfined())
        {
            int result = (int) NativeCalls.invoke(ElementFunctions.HAS_ATTRIBUTE, handle(), NativeMemory.text(arena, name));

            if (result < 0)
                throw NativeCalls.lastError();
            return result == 1;
        }
    }

    /** true — атрибут был и удалён, false — его не было. */
    default boolean removeAttribute(String name)
    {
        try (Arena arena = Arena.ofConfined())
        {
            int result = (int) NativeCalls.invoke(ElementFunctions.REMOVE_ATTRIBUTE, handle(), NativeMemory.text(arena, name));

            if (result < 0)
                throw NativeCalls.lastError();
            return result == 1;
        }
    }
}
