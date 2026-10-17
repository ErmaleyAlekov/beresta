package org.beresta;

import java.lang.foreign.Arena;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.XPathFunctions;

/**
 * Контекст выражения: префиксы пространств имён и переменные $name.
 * Методы возвращают само выражение — их удобно вызывать цепочкой.
 */
public interface XPathVariables extends XPathBase
{
    /** Префикс для имён вида p:name в выражении. */
    default XPath bind(String prefix, String uri)
    {
        try (Arena arena = Arena.ofConfined())
        {
            NativeCalls.check(NativeCalls.invoke(XPathFunctions.BIND_NAMESPACE, handle(),
                NativeMemory.text(arena, prefix), NativeMemory.text(arena, uri)));
            return (XPath) this;
        }
    }

    /** Строковая переменная. */
    default XPath set(String name, String value)
    {
        try (Arena arena = Arena.ofConfined())
        {
            NativeCalls.check(NativeCalls.invoke(XPathFunctions.SET_STRING, handle(),
                NativeMemory.text(arena, name), NativeMemory.text(arena, value)));
            return (XPath) this;
        }
    }

    /** Числовая переменная. */
    default XPath set(String name, double value)
    {
        try (Arena arena = Arena.ofConfined())
        {
            NativeCalls.check(NativeCalls.invoke(XPathFunctions.SET_NUMBER, handle(), NativeMemory.text(arena, name), value));
            return (XPath) this;
        }
    }
}
