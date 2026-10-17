package org.beresta.internal;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;

/**
 * Подготовка аргументов в нативной памяти. Память берётся из Arena,
 * переданной вызывающим (обычно Arena.ofConfined() в try-with-resources),
 * и освобождается сразу после вызова — Java работает с памятью напрямую,
 * без копирования через JNI.
 */
public final class NativeMemory
{
    private NativeMemory()
    {
    }

    /** Строка Java -> const char* в UTF-8 (null -> NULL). */
    public static MemorySegment text(Arena arena, String value)
    {
        if (value == null)
            return MemorySegment.NULL;
        return arena.allocateFrom(value);
    }

    /** Байты -> нативный буфер (для разбора документа в любой кодировке). */
    public static MemorySegment bytes(Arena arena, byte[] data)
    {
        MemorySegment buffer = arena.allocate(Math.max(1, data.length));

        MemorySegment.copy(data, 0, buffer, ValueLayout.JAVA_BYTE, 0, data.length);
        return buffer;
    }

    /** Ячейка для выходного параметра вида beresta_document* out. */
    public static MemorySegment outPointer(Arena arena)
    {
        return arena.allocate(ValueLayout.ADDRESS);
    }

    /** Значение, записанное библиотекой в выходной параметр. */
    public static MemorySegment readPointer(MemorySegment out)
    {
        return out.get(ValueLayout.ADDRESS, 0);
    }
}
