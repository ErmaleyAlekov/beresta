package org.beresta.internal;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;

/**
 * Подготовка аргументов в нативной памяти. Память берётся из Arena,
 * переданной вызывающим (обычно Arena.ofConfined() в try-with-resources),
 * и освобождается сразу после вызова — Java работает с памятью напрямую,
 * без копирования через JNI.
 *
 * <p>Prepares arguments in native memory. The memory comes from the Arena passed
 * by the caller (usually Arena.ofConfined() in try-with-resources) and is freed
 * right after the call — Java works with the memory directly, without copying
 * through JNI.
 */
public final class NativeMemory
{
    private NativeMemory()
    {
    }

    /** Строка Java -> const char* в UTF-8 (null -> NULL). / A Java string -> a UTF-8 const char* (null -> NULL). */
    public static MemorySegment text(Arena arena, String value)
    {
        if (value == null)
            return MemorySegment.NULL;
        return arena.allocateFrom(value);
    }

    /**
     * Байты -> нативный буфер (для разбора документа в любой кодировке).
     * <p>Bytes -> a native buffer (to parse a document in any encoding).
     */
    public static MemorySegment bytes(Arena arena, byte[] data)
    {
        MemorySegment buffer = arena.allocate(Math.max(1, data.length));

        MemorySegment.copy(data, 0, buffer, ValueLayout.JAVA_BYTE, 0, data.length);
        return buffer;
    }

    /**
     * Ячейка для выходного параметра вида beresta_document* out.
     * <p>A cell for an output parameter like beresta_document* out.
     */
    public static MemorySegment outPointer(Arena arena)
    {
        return arena.allocate(ValueLayout.ADDRESS);
    }

    /**
     * Значение, записанное библиотекой в выходной параметр.
     * <p>The value the library wrote into the output parameter.
     */
    public static MemorySegment readPointer(MemorySegment out)
    {
        return out.get(ValueLayout.ADDRESS, 0);
    }
}
