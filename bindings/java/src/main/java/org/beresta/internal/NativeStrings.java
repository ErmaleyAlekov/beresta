package org.beresta.internal;

import java.lang.foreign.MemorySegment;

/**
 * Чтение строк, которые вернула библиотека. Две разновидности по
 * соглашению Си-интерфейса: заимствованная const char* (читается и
 * остаётся у библиотеки) и собственная char* (читается и освобождается).
 */
public final class NativeStrings
{
    private NativeStrings()
    {
    }

    /**
     * Заимствованная строка (const char*): beresta_version, имя кода,
     * сообщение об ошибке. Указатель от FFM API имеет нулевой размер —
     * reinterpret расширяет его, чтобы дочитать строку до нуля.
     */
    public static String borrowed(Object result)
    {
        return ((MemorySegment) result).reinterpret(Long.MAX_VALUE).getString(0);
    }

    /**
     * Собственная строка (char*): копируется в String (UTF-8) и сразу
     * освобождается через beresta_string_free — утечки нет.
     */
    public static String take(Object result)
    {
        MemorySegment pointer = NativeCalls.handle(result);

        if (pointer == null)
            return null;
        try
        {
            return borrowed(pointer);
        }
        finally
        {
            NativeCalls.invoke(LibraryFunctions.STRING_FREE, pointer);
        }
    }
}
