package org.beresta;

import org.beresta.internal.LibraryFunctions;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeStrings;

/**
 * Ошибка «Берёсты». Код совпадает с beresta_status из Си-интерфейса,
 * имя кода (например, PARSE_ERROR) одинаково во всех языках.
 * Непроверяемое исключение: как и в DOM-API JDK, ошибки XML — повод
 * прервать операцию, а не обязательный пункт сигнатуры каждого метода.
 */
public final class BerestaException extends RuntimeException
{
    private static final long serialVersionUID = 1L;

    private final int code;
    private final String codeName;
    private final String detail;

    public BerestaException(int code, String detail)
    {
        super("[" + nameOf(code) + "] " + detail);
        this.code = code;
        this.codeName = nameOf(code);
        this.detail = detail;
    }

    /** Имя кода состояния из библиотеки (beresta_status_name). */
    private static String nameOf(int code)
    {
        return NativeStrings.borrowed(NativeCalls.invoke(LibraryFunctions.STATUS_NAME, code));
    }

    /** Числовой код (beresta_status). */
    public int code()
    {
        return code;
    }

    /** Имя кода: PARSE_ERROR, VALIDATION_ERROR, INVALID_NAME... */
    public String codeName()
    {
        return codeName;
    }

    /** Текст ошибки без префикса с кодом. */
    public String detail()
    {
        return detail;
    }
}
