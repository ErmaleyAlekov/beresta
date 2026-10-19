package org.beresta;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.nio.file.Path;
import java.util.List;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.SchemaFunctions;

/**
 * Проверка по схеме — примесь XmlSchema (интерфейс с методами по
 * умолчанию, как XPathEvaluation у XPath). Два способа:
 * <ul>
 *   <li>{@code validate(document)} — готовое дерево;</li>
 *   <li>{@code validate(bytes, flags)} и {@code validateFile(path)} — потоково,
 *   без дерева (SAX + XSD внутри ядра): память — O(глубины документа),
 *   файл отображается в память, а не копируется. Так проверяются выгрузки
 *   в гигабайты, которые в дерево не помещаются.</li>
 * </ul>
 * Нарушения схемы возвращаются списком; синтаксическая ошибка XML —
 * исключение BerestaException(PARSE_ERROR).
 *
 * <p>Schema validation — a mixin of XmlSchema (an interface with default
 * methods, like XPathEvaluation for XPath). Two ways:
 * <ul>
 *   <li>{@code validate(document)} — a ready tree;</li>
 *   <li>{@code validate(bytes, flags)} and {@code validateFile(path)} — streaming,
 *   without a tree (SAX + XSD inside the core): the memory is O(document depth),
 *   the file is mapped into memory, not copied. This is how exports of
 *   gigabytes that do not fit into a tree are validated.</li>
 * </ul>
 * Schema violations are returned as a list; an XML syntax error is a
 * BerestaException(PARSE_ERROR).
 */
public interface SchemaValidation
{
    /** Нативный дескриптор схемы. / The native handle of the schema. */
    MemorySegment handle();

    /**
     * Все нарушения готового дерева за один проход; пустой список — документ корректен.
     * <p>All violations of a ready tree in one pass; an empty list means the document is valid.
     */
    default List<Diagnostic> validate(XmlDocument document)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(SchemaFunctions.VALIDATE, handle(), document.handle(), out));
            return DiagnosticReader.take(out);
        }
    }

    /**
     * Потоковая проверка байтов в любой поддерживаемой кодировке; flags — ParseFlags.
     * <p>Streaming validation of bytes in any supported encoding; flags are ParseFlags.
     */
    default List<Diagnostic> validate(byte[] data, int flags)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(SchemaFunctions.VALIDATE_BYTES, NativeMemory.bytes(arena, data),
                (long) data.length, flags, handle(), out));
            return DiagnosticReader.take(out);
        }
    }

    /**
     * Потоковая проверка файла; ParseFlags.FILE_RESOURCES разрешает внешние DTD из его каталога.
     * <p>Streaming validation of a file; ParseFlags.FILE_RESOURCES allows external DTDs from its directory.
     */
    default List<Diagnostic> validateFile(Path path, int flags)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(SchemaFunctions.VALIDATE_FILE,
                NativeMemory.text(arena, path.toAbsolutePath().toString()), flags, handle(), out));
            return DiagnosticReader.take(out);
        }
    }

    /**
     * Потоковая проверка файла с флагами по умолчанию.
     * <p>Streaming validation of a file with the default flags.
     */
    default List<Diagnostic> validateFile(Path path)
    {
        return validateFile(path, ParseFlags.DEFAULT);
    }
}
