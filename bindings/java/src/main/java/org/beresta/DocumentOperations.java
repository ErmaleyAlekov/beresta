package org.beresta;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.nio.file.Path;

import org.beresta.internal.DocumentFunctions;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;

/**
 * Операции уровня документа: корень, поиск по ID, копия, запись в файл.
 * <p>Document-level operations: the root, lookup by ID, copying, writing to a file.
 */
public interface DocumentOperations extends NodeBase
{
    /** Корневой элемент или null. / The root element or null. */
    default XmlElement root()
    {
        return (XmlElement) NodeFactory.wrap(NativeCalls.invoke(DocumentFunctions.ROOT, handle()), document());
    }

    /**
     * Элемент с атрибутом типа ID (по DTD) или xml:id; null — не найден.
     * <p>An element with an ID-typed attribute (by DTD) or xml:id; null if not found.
     */
    default XmlElement elementById(String id)
    {
        try (Arena arena = Arena.ofConfined())
        {
            Object pointer = NativeCalls.invoke(DocumentFunctions.ELEMENT_BY_ID, handle(), NativeMemory.text(arena, id));

            return (XmlElement) NodeFactory.wrap(pointer, document());
        }
    }

    /** Независимая глубокая копия; её тоже нужно закрыть. / An independent deep copy; it must be closed too. */
    default XmlDocument copy()
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(DocumentFunctions.CLONE, handle(), out));
            return new XmlDocument(NativeMemory.readPointer(out));
        }
    }

    /** Запись в UTF-8 без лишних пробелов. / Writes UTF-8 without extra spaces. */
    default void save(Path path)
    {
        save(path, WriteFlags.DEFAULT, null);
    }

    /**
     * Запись с флагами WriteFlags; encoding == null — UTF-8, иначе, например, "windows-1251".
     * <p>Writes with WriteFlags; encoding == null means UTF-8, otherwise, for example, "windows-1251".
     */
    default void save(Path path, int flags, String encoding)
    {
        try (Arena arena = Arena.ofConfined())
        {
            NativeCalls.check(NativeCalls.invoke(DocumentFunctions.SAVE, handle(),
                NativeMemory.text(arena, path.toAbsolutePath().toString()), flags, NativeMemory.text(arena, encoding)));
        }
    }
}
