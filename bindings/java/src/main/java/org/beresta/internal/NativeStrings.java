package org.beresta.internal;

import java.lang.foreign.MemorySegment;

/**
 * Чтение строк, которые вернула библиотека. Две разновидности по
 * соглашению Си-интерфейса: заимствованная const char* (читается и
 * остаётся у библиотеки) и собственная char* (читается и освобождается).
 *
 * <p>Reads strings returned by the library. Two kinds by the C interface
 * convention: a borrowed const char* (read, stays with the library)
 * and an owned char* (read and freed).
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
     *
     * <p>A borrowed string (const char*): beresta_version, a code name,
     * an error message. A pointer from the FFM API has zero size —
     * reinterpret widens it so the string can be read up to the terminator.
     */
    public static String borrowed(Object result)
    {
        return ((MemorySegment) result).reinterpret(Long.MAX_VALUE).getString(0);
    }

    /**
     * Сообщение обёртки на текущем языке из каталога библиотеки (beresta_message),
     * аргументы подставляются вместо {0}, {1}...
     * A binding message in the current language from the library catalog
     * (beresta_message); arguments replace {0}, {1}...
     */
    public static String message(int message, Object... arguments)
    {
        String text = borrowed(NativeCalls.invoke(LibraryFunctions.MESSAGE_TEXT, message));

        for (int index = 0; index < arguments.length; ++index)
            text = text.replace("{" + index + "}", String.valueOf(arguments[index]));
        return text;
    }

    /**
     * Собственная строка (char*): копируется в String (UTF-8) и сразу
     * освобождается через beresta_string_free — утечки нет.
     *
     * <p>An owned string (char*): copied into a String (UTF-8) and freed at once
     * through beresta_string_free — no leaks.
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
