package org.beresta;

import org.beresta.internal.LibraryFunctions;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeStrings;

/**
 * Ошибка «Берёсты». Код совпадает с beresta_status из Си-интерфейса,
 * имя кода (например, PARSE_ERROR) одинаково во всех языках.
 * Непроверяемое исключение: как и в DOM-API JDK, ошибки XML — повод
 * прервать операцию, а не обязательный пункт сигнатуры каждого метода.
 *
 * <p>A Beresta error. The code matches beresta_status of the C interface,
 * the code name (for example, PARSE_ERROR) is the same in all languages.
 * An unchecked exception: as in the JDK DOM API, XML errors are a reason to
 * abort the operation, not a mandatory part of every method signature.
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

    /**
     * Имя кода состояния из библиотеки (beresta_status_name).
     * <p>The status code name from the library (beresta_status_name).
     */
    private static String nameOf(int code)
    {
        return NativeStrings.borrowed(NativeCalls.invoke(LibraryFunctions.STATUS_NAME, code));
    }

    /** Числовой код (beresta_status). / The numeric code (beresta_status). */
    public int code()
    {
        return code;
    }

    /**
     * Имя кода: PARSE_ERROR, VALIDATION_ERROR, INVALID_NAME...
     * <p>The code name: PARSE_ERROR, VALIDATION_ERROR, INVALID_NAME...
     */
    public String codeName()
    {
        return codeName;
    }

    /** Текст ошибки без префикса с кодом. / The error text without the code prefix. */
    public String detail()
    {
        return detail;
    }
}
