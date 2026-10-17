using System;
using System.Runtime.InteropServices;

namespace Beresta.Native
{
    /// <summary>
    /// Объявления функций Си-интерфейса (зеркало include/capi/beresta_c.h).
    /// Класс разбит на partial-файлы по разделам заголовка.
    ///
    /// Соглашения маршалинга:
    ///   const char* (вход)  — string с UnmanagedType.LPUTF8Str (UTF-8 без копий вручную);
    ///   char* (выход)       — IntPtr: строку нужно освободить beresta_string_free,
    ///                          автоматический маршалинг string её бы потерял;
    ///   дескрипторы         — IntPtr; int64_t — long; uint32_t — uint.
    /// </summary>
    internal static partial class NativeMethods
    {
        /// <summary>Имя библиотеки: .NET сам добавит lib/.so/.dylib/.dll по ОС.</summary>
        internal const string Library = "beresta";

#if NET5_0_OR_GREATER
        /// <summary>
        /// Статический конструктор выполняется до первого обращения к любому
        /// члену класса, то есть до первого P/Invoke — самое раннее и при этом
        /// гарантированное место, чтобы подключить собственный поиск библиотеки.
        /// </summary>
        static NativeMethods()
        {
            NativeResolver.Register();
        }
#endif

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_version();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_last_error_code();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_last_error_message();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_status_name(int status);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern void beresta_string_free(IntPtr text);
    }
}
