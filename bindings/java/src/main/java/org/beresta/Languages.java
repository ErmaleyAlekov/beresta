package org.beresta;

import java.lang.foreign.Arena;
import java.util.List;

import org.beresta.internal.LibraryFunctions;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.NativeStrings;

/**
 * Язык сообщений об ошибках. По умолчанию — язык, выбранный при сборке
 * нативной библиотеки (ERRORS_LANG); сменить можно в любой момент:
 * {@code Languages.use("en")}.
 *
 * <p>Language of error messages. The default is the language chosen when the
 * native library was built (ERRORS_LANG); it can be changed at any time:
 * {@code Languages.use("en")}.
 */
public final class Languages
{
    private Languages()
    {
    }

    /** Код текущего языка, например "ru". / The current language code, for example "ru". */
    public static String current()
    {
        return NativeStrings.borrowed(NativeCalls.invoke(LibraryFunctions.LANGUAGE));
    }

    /**
     * Смена языка для всех потоков; неизвестный код — BerestaException(INVALID_ARGUMENT).
     * Changes the language for all threads; an unknown code throws BerestaException(INVALID_ARGUMENT).
     */
    public static void use(String code)
    {
        try (Arena arena = Arena.ofConfined())
        {
            NativeCalls.check(NativeCalls.invoke(LibraryFunctions.SET_LANGUAGE, NativeMemory.text(arena, code)));
        }
    }

    /** Все языки библиотеки. / All languages of the library. */
    public static List<String> available()
    {
        return List.of(NativeStrings.borrowed(NativeCalls.invoke(LibraryFunctions.LANGUAGES)).split(","));
    }
}
