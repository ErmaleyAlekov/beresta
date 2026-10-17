package org.beresta;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.ref.Cleaner;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeStrings;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.NativeRelease;
import org.beresta.internal.SchemaFunctions;

/**
 * Загруженная XML Schema. После загрузки неизменяема: одну схему можно
 * использовать для множества документов, в том числе из разных потоков.
 */
public final class XmlSchema implements AutoCloseable
{
    final MemorySegment pointer;
    private final Cleaner.Cleanable cleanable;

    private XmlSchema(MemorySegment out)
    {
        this.pointer = NativeMemory.readPointer(out);
        this.cleanable = NativeRelease.register(this, SchemaFunctions.FREE, pointer);
    }

    /** Схема из текста XSD (xs:include здесь недоступен — нет каталога). */
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

    /** Схема из файла; xs:include / xs:import ищутся в его каталоге. */
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

    /** Все нарушения за один проход; пустой список — документ корректен. */
    public List<Diagnostic> validate(XmlDocument document)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);
            NativeCalls.check(NativeCalls.invoke(SchemaFunctions.VALIDATE, pointer, document.handle(), out));
            MemorySegment list = NativeMemory.readPointer(out);
            List<Diagnostic> result = new ArrayList<>();
            try
            {
                long count = (long) NativeCalls.invoke(SchemaFunctions.DIAGNOSTICS_COUNT, list);
                for (long index = 0; index < count; ++index)
                    result.add(new Diagnostic(
                        NativeStrings.take(NativeCalls.invoke(SchemaFunctions.DIAGNOSTICS_MESSAGE, list, index)),
                        (long) NativeCalls.invoke(SchemaFunctions.DIAGNOSTICS_LINE, list, index),
                        (long) NativeCalls.invoke(SchemaFunctions.DIAGNOSTICS_COLUMN, list, index)));
                return result;
            }
            finally
            {
                NativeCalls.invoke(SchemaFunctions.DIAGNOSTICS_FREE, list);
            }
        }
    }

    @Override
    public void close()
    {
        cleanable.clean();
    }
}
