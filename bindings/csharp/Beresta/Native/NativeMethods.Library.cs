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
    /// Declarations of the C interface functions (a mirror of include/capi/beresta_c.h).
    /// The class is split into partial files by the header sections.
    /// 
    /// Marshalling conventions:
    ///   const char* (input)  — string with UnmanagedType.LPUTF8Str (UTF-8 without manual copies);
    ///   char* (output)       — IntPtr: the string must be freed with beresta_string_free,
    ///                          automatic string marshalling would lose it;
    ///   handles              — IntPtr; int64_t — long; uint32_t — uint.
    /// </summary>
    internal static partial class NativeMethods
    {
        /// <summary>Имя библиотеки: .NET сам добавит lib/.so/.dylib/.dll по ОС.
        /// The library name: .NET adds lib/.so/.dylib/.dll for the OS itself.</summary>
        internal const string Library = "beresta";

#if NET5_0_OR_GREATER
        /// <summary>
        /// Статический конструктор выполняется до первого обращения к любому
        /// члену класса, то есть до первого P/Invoke — самое раннее и при этом
        /// гарантированное место, чтобы подключить собственный поиск библиотеки.
        /// The static constructor runs before the first access to any member of the
        /// class, that is, before the first P/Invoke — the earliest and guaranteed place
        /// to plug in our own library lookup.
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

        // ---- Языки сообщений / Message languages ----

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_set_language([MarshalAs(UnmanagedType.LPUTF8Str)] string code);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_language();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_languages();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_message_text(int message);
    }
}
