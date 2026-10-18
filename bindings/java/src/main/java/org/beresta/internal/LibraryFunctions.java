package org.beresta.internal;

import static org.beresta.internal.Layouts.INT;
import static org.beresta.internal.Layouts.POINTER;

import java.lang.foreign.FunctionDescriptor;
import java.lang.invoke.MethodHandle;

/**
 * Функции библиотеки и ошибок (раздел «Библиотека и ошибки» beresta_c.h).
 * <p>Library and error functions (the "Library and errors" section of beresta_c.h).
 */
public final class LibraryFunctions
{
    public static final MethodHandle VERSION =
        NativeLibrary.function("beresta_version", FunctionDescriptor.of(POINTER));
    public static final MethodHandle LAST_ERROR_CODE =
        NativeLibrary.function("beresta_last_error_code", FunctionDescriptor.of(INT));
    public static final MethodHandle LAST_ERROR_MESSAGE =
        NativeLibrary.function("beresta_last_error_message", FunctionDescriptor.of(POINTER));
    public static final MethodHandle STATUS_NAME =
        NativeLibrary.function("beresta_status_name", FunctionDescriptor.of(POINTER, INT));
    public static final MethodHandle STRING_FREE =
        NativeLibrary.function("beresta_string_free", FunctionDescriptor.ofVoid(POINTER));

    // Языки сообщений / Message languages
    public static final MethodHandle SET_LANGUAGE =
        NativeLibrary.function("beresta_set_language", FunctionDescriptor.of(INT, POINTER));
    public static final MethodHandle LANGUAGE =
        NativeLibrary.function("beresta_language", FunctionDescriptor.of(POINTER));
    public static final MethodHandle LANGUAGES =
        NativeLibrary.function("beresta_languages", FunctionDescriptor.of(POINTER));
    public static final MethodHandle MESSAGE_TEXT =
        NativeLibrary.function("beresta_message_text", FunctionDescriptor.of(POINTER, INT));

    private LibraryFunctions()
    {
    }
}
