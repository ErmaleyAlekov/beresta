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
 *
 * <p>An XML document — the owner of the whole node tree in native memory.
 * The document is itself a node (children can be added to it) and implements
 * AutoCloseable: use try-with-resources. If the document was not closed,
 * Cleaner frees the memory on garbage collection.
 */
public final class XmlDocument extends XmlNode implements DocumentOperations, AutoCloseable
{
    private final long address;

    // Единственные обёртки узлов этого документа (см. NodeRegistry)
    // The only wrappers of the nodes of this document (see NodeRegistry)
    final NodeRegistry registry = new NodeRegistry();
    private final Cleaner.Cleanable cleanable;
    private volatile boolean closed;

    XmlDocument(MemorySegment pointer)
    {
        super(pointer, null);
        this.address = pointer.address();
        this.cleanable = NativeRelease.register(this, DocumentFunctions.FREE, pointer);
    }

    /**
     * Адрес без проверки закрытия — для фабрики обёрток.
     * <p>The address without the closed check — for the wrapper factory.
     */
    long address()
    {
        return address;
    }

    public boolean isClosed()
    {
        return closed;
    }

    /**
     * Освобождение нативной памяти; повторный вызов безопасен.
     * <p>Releases the native memory; calling it again is safe.
     */
    @Override
    public void close()
    {
        closed = true;
        cleanable.clean();
    }

    /**
     * Новый документ; rootName == null — без корневого элемента.
     * <p>A new document; rootName == null means without a root element.
     */
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
