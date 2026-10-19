package org.beresta;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.ref.Cleaner;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.NativeRelease;
import org.beresta.internal.SchemaFunctions;

/**
 * Загруженная XML Schema. После загрузки неизменяема: одну схему можно
 * использовать для множества документов, в том числе из разных потоков.
 * Методы проверки — в примеси {@link SchemaValidation}: готовое дерево
 * ({@code validate(document)}) или поток без дерева ({@code validate(bytes, flags)},
 * {@code validateFile(path)}).
 *
 * <p>A loaded XML Schema. Immutable after loading: one schema can be used
 * for many documents, including from different threads.
 * The validation methods are in the {@link SchemaValidation} mixin: a ready tree
 * ({@code validate(document)}) or a stream without a tree ({@code validate(bytes, flags)},
 * {@code validateFile(path)}).
 */
public final class XmlSchema implements SchemaValidation, AutoCloseable
{
    final MemorySegment pointer;
    private final Cleaner.Cleanable cleanable;

    private XmlSchema(MemorySegment out)
    {
        this.pointer = NativeMemory.readPointer(out);
        this.cleanable = NativeRelease.register(this, SchemaFunctions.FREE, pointer);
    }

    /**
     * Схема из текста XSD (xs:include здесь недоступен — нет каталога).
     * <p>A schema from XSD text (xs:include is unavailable here — there is no directory).
     */
    public static XmlSchema fromString(String xsd)
    {
        byte[] data = xsd.getBytes(StandardCharsets.UTF_8);

        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(SchemaFunctions.LOAD, NativeMemory.bytes(arena, data), (long) data.length, out));
            return new XmlSchema(out);
        }
    }

    /**
     * Схема из файла; xs:include / xs:import ищутся в его каталоге.
     * <p>A schema from a file; xs:include / xs:import are looked up in its directory.
     */
    public static XmlSchema fromFile(Path path)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(SchemaFunctions.LOAD_FILE,
                NativeMemory.text(arena, path.toAbsolutePath().toString()), out));
            return new XmlSchema(out);
        }
    }

    /** Нативный дескриптор схемы. / The native handle of the schema. */
    @Override
    public MemorySegment handle()
    {
        return pointer;
    }

    @Override
    public void close()
    {
        cleanable.clean();
    }
}
