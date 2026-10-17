package org.beresta.internal;

import java.lang.foreign.MemorySegment;
import java.lang.invoke.MethodHandle;

import org.beresta.BerestaException;

/**
 * Единый способ вызвать Си-функцию и обработать её результат.
 *
 * <p>Соглашения Си-интерфейса одинаковы для всех функций (статус 0 — успех,
 * NULL с ненулевым кодом ошибки — сбой, char* освобождается библиотекой),
 * поэтому их обработка написана ровно один раз (DRY).
 */
public final class NativeCalls
{
    private NativeCalls()
    {
    }

    /**
     * Вызов Си-функции. invokeWithArguments сам приводит упакованные
     * аргументы к сигнатуре; проверяемый Throwable превращается в
     * исключение времени выполнения, ошибки JVM пробрасываются как есть.
     */
    public static Object invoke(MethodHandle function, Object... arguments)
    {
        try
        {
            return function.invokeWithArguments(arguments);
        }
        catch (RuntimeException | Error error)
        {
            throw error;
        }
        catch (Throwable error)
        {
            throw new IllegalStateException("Берёста: сбой нативного вызова", error);
        }
    }

    /** Исключение по последней ошибке текущего потока. */
    public static BerestaException lastError()
    {
        int code = (int) invoke(LibraryFunctions.LAST_ERROR_CODE);
        String message = NativeStrings.borrowed(invoke(LibraryFunctions.LAST_ERROR_MESSAGE));

        return new BerestaException(code, message);
    }

    /** Проверка int-статуса: всё, кроме 0, — исключение. */
    public static void check(Object status)
    {
        if ((int) status != 0)
            throw lastError();
    }

    /**
     * Проверка дескриптора-результата: NULL без ошибки — законное «нет
     * объекта» (результат null), NULL с ошибкой — исключение.
     */
    public static MemorySegment handle(Object result)
    {
        MemorySegment pointer = (MemorySegment) result;

        if (pointer.address() != 0)
            return pointer;
        if ((int) invoke(LibraryFunctions.LAST_ERROR_CODE) != 0)
            throw lastError();
        return null;
    }
}
