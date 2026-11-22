package org.beresta.internal;

import java.lang.foreign.MemorySegment;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

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
    private static final Pattern PLACEHOLDER = Pattern.compile("\\{([0-9]+)\\}");

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
        MemorySegment pointer = (MemorySegment) result;
        if (pointer == null || pointer.address() == 0)
            throw new IllegalStateException("Берёста: строка отсутствует / Beresta: missing native string");
        return pointer.reinterpret(Long.MAX_VALUE).getString(0);
    }

    /**
     * Сообщение обёртки на текущем языке из каталога библиотеки (beresta_message),
     * аргументы подставляются вместо {0}, {1}...
     * A binding message in the current language from the library catalog
     * (beresta_message); arguments replace {0}, {1}...
     */
    public static String message(int message, Object... arguments)
    {
        String text = borrowed(NativeCalls.handle(NativeCalls.invoke(LibraryFunctions.MESSAGE_TEXT, message)));

        return PLACEHOLDER.matcher(text).replaceAll(match -> {
            int index = Integer.parseInt(match.group(1));
            String value = index < arguments.length ? String.valueOf(arguments[index]) : match.group();
            return Matcher.quoteReplacement(value);
        });
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
