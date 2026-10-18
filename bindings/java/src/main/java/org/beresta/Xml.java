package org.beresta;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;

import org.beresta.internal.DocumentFunctions;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;

/**
 * Точка входа (паттерн Facade): разбор текста, байтов и файлов.
 * Новый пустой документ создаёт XmlDocument.create. Все методы
 * потокобезопасны — каждый вызов создаёт собственный нативный документ.
 *
 * <p>The entry point (the Facade pattern): parsing text, bytes and files.
 * XmlDocument.create creates a new empty document. All methods are
 * thread-safe — each call creates its own native document.
 */
public final class Xml
{
    private Xml()
    {
    }

    /** Разбор строки с настройками по умолчанию. / Parses a string with default settings. */
    public static XmlDocument parse(String xml)
    {
        return parse(xml.getBytes(StandardCharsets.UTF_8), ParseFlags.DEFAULT, null);
    }

    /**
     * Разбор строки с флагами ParseFlags и (необязательно) проверкой по схеме.
     * <p>Parses a string with ParseFlags and (optionally) schema validation.
     */
    public static XmlDocument parse(String xml, int flags, XmlSchema schema)
    {
        return parse(xml.getBytes(StandardCharsets.UTF_8), flags, schema);
    }

    /**
     * Разбор байтов в любой поддерживаемой кодировке (UTF-8/16,
     * windows-1251, KOI8-R...): её определит библиотека по BOM и декларации.
     *
     * <p>Parses bytes in any supported encoding (UTF-8/16,
     * windows-1251, KOI8-R...): the library detects it by the BOM and the declaration.
     */
    public static XmlDocument parse(byte[] data, int flags, XmlSchema schema)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(DocumentFunctions.PARSE, NativeMemory.bytes(arena, data),
                (long) data.length, flags, NodeFactory.schemaHandle(schema), out));
            return new XmlDocument(NativeMemory.readPointer(out));
        }
    }

    /**
     * Разбор файла; ParseFlags.FILE_RESOURCES разрешает внешние DTD из его каталога.
     * <p>Parses a file; ParseFlags.FILE_RESOURCES allows external DTDs from its directory.
     */
    public static XmlDocument parseFile(Path path, int flags, XmlSchema schema)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(DocumentFunctions.PARSE_FILE,
                NativeMemory.text(arena, path.toAbsolutePath().toString()), flags, NodeFactory.schemaHandle(schema), out));
            return new XmlDocument(NativeMemory.readPointer(out));
        }
    }
}
