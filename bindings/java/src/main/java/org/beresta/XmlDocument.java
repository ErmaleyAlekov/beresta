package org.beresta;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.ref.Cleaner;

import org.beresta.internal.DocumentFunctions;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.NativeRelease;

/**
 * Документ XML — владелец всего дерева узлов в нативной памяти.
 *
 * <p>Документ сам является узлом (к нему можно добавлять детей) и
 * реализует AutoCloseable: используйте try-with-resources. Если документ
 * забыли закрыть, память освободит Cleaner при сборке мусора.
 *
 * <pre>{@code
 * try (XmlDocument document = Xml.parse("<shop><item>Чай</item></shop>"))
 * {
 *     System.out.println(document.root().firstChild().text());
 * }
 * }</pre>
 */
public final class XmlDocument extends XmlNode implements DocumentOperations, AutoCloseable
{
    private final long address;
    private final Cleaner.Cleanable cleanable;
    private volatile boolean closed;

    XmlDocument(MemorySegment pointer)
    {
        super(pointer, null);
        this.address = pointer.address();
        this.cleanable = NativeRelease.register(this, DocumentFunctions.FREE, pointer);
    }

    /** Адрес без проверки закрытия — для фабрики обёрток. */
    long address()
    {
        return address;
    }

    public boolean isClosed()
    {
        return closed;
    }

    /** Освобождение нативной памяти; повторный вызов безопасен. */
    @Override
    public void close()
    {
        closed = true;
        cleanable.clean();
    }

    /** Новый документ; rootName == null — без корневого элемента. */
    public static XmlDocument create(String rootName)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(DocumentFunctions.CREATE, NativeMemory.text(arena, rootName), out));
            return new XmlDocument(NativeMemory.readPointer(out));
        }
    }
}
