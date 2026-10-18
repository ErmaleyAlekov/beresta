package org.beresta;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.ref.Cleaner;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.NativeRelease;
import org.beresta.internal.NativeStrings;
import org.beresta.internal.XPathFunctions;

/**
 * Скомпилированное выражение XPath 1.0: компилируется один раз,
 * вычисляется сколько угодно раз для разных узлов и документов.
 *
 * <pre>{@code
 * try (XPath query = new XPath("//s:item[@code = $code]"))
 * {
 *     query.bind("s", "urn:shop").set("code", "B2");
 *     String name = query.string(document);
 * }
 * }</pre>
 */
public final class XPath implements XPathVariables, XPathEvaluation, AutoCloseable
{
    private final MemorySegment pointer;
    private final Cleaner.Cleanable cleanable;
    private volatile boolean closed;

    /**
     * Компиляция; синтаксическая ошибка — BerestaException(XPATH_ERROR).
     * <p>Compilation; a syntax error throws BerestaException(XPATH_ERROR).
     */
    public XPath(String expression)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(XPathFunctions.COMPILE, NativeMemory.text(arena, expression), out));
            pointer = NativeMemory.readPointer(out);
        }
        cleanable = NativeRelease.register(this, XPathFunctions.FREE, pointer);
    }

    @Override
    public MemorySegment handle()
    {
        if (closed)
            throw new IllegalStateException("Берёста / Beresta: " + NativeStrings.message(2));
        return pointer;
    }

    @Override
    public void close()
    {
        closed = true;
        cleanable.clean();
    }
}
