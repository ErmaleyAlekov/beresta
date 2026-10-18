using System;
using System.Runtime.InteropServices;

namespace Beresta.Native
{
    /// <summary>
    /// Обработка результатов Си-функций по единым соглашениям интерфейса
    /// (статус 0 — успех, NULL с ненулевым кодом — сбой, char* освобождается
    /// библиотекой). Написано один раз для всех вызовов (DRY).
    /// Handles the results of C functions by the common interface conventions
    /// (status 0 is success, NULL with a non-zero code is a failure, char* is freed
    /// by the library). Written once for all calls (DRY).
    /// </summary>
    internal static class NativeCalls
    {
        /// <summary>Исключение по последней ошибке текущего потока.
        /// An exception for the last error of the current thread.</summary>
        internal static BerestaException LastError()
        {
            int code = NativeMethods.beresta_last_error_code();
            string message = Marshal.PtrToStringUTF8(NativeMethods.beresta_last_error_message()) ?? string.Empty;

            return new BerestaException(code, message);
        }

        /// <summary>Проверка int-статуса: всё, кроме 0, — исключение.
        /// Checks an int status: anything except 0 is an exception.</summary>
        internal static void Check(int status)
        {
            if (status != 0)
                throw LastError();
        }

        /// <summary>
        /// Дескриптор-результат: NULL без ошибки — законное «объекта нет»
        /// (IntPtr.Zero), NULL с ошибкой — исключение.
        /// A handle result: NULL without an error is a legitimate "no object"
        /// (IntPtr.Zero), NULL with an error is an exception.
        /// </summary>
        internal static IntPtr Handle(IntPtr pointer)
        {
            if (pointer == IntPtr.Zero && NativeMethods.beresta_last_error_code() != 0)
                throw LastError();
            return pointer;
        }

        /// <summary>
        /// Строка char*, принадлежащая вызывающему: копируется в string
        /// и сразу освобождается — утечки неуправляемой памяти нет.
        /// A char* string owned by the caller: copied into a string
        /// and freed at once — no unmanaged memory leaks.
        /// </summary>
        internal static string? TakeString(IntPtr pointer)
        {
            if (Handle(pointer) == IntPtr.Zero)
                return null;
            try
            {
                return Marshal.PtrToStringUTF8(pointer);
            }
            finally
            {
                NativeMethods.beresta_string_free(pointer);
            }
        }

        /// <summary>Строка, которая по контракту функции не бывает NULL без ошибки.
        /// A string that by the function contract is never NULL without an error.</summary>
        internal static string TakeRequiredString(IntPtr pointer)
        {
            string? text = TakeString(pointer);

            if (text == null)
                throw LastError();
            return text;
        }
    }
}
