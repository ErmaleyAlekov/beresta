using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Сведения о нативной библиотеке и язык сообщений об ошибках.
    /// Information about the native library and the language of error messages.
    /// </summary>
    public static class BerestaLibrary
    {
        /// <summary>
        /// Версия нативной библиотеки, например "2.1.0".
        /// The native library version, for example "2.1.0".
        /// </summary>
        public static string Version
        {
            get { return Marshal.PtrToStringUTF8(NativeMethods.beresta_version()) ?? string.Empty; }
        }

        /// <summary>
        /// Язык сообщений для всех потоков ("ru", "en"...). По умолчанию — язык сборки
        /// (ERRORS_LANG); неизвестный код — BerestaException(INVALID_ARGUMENT).
        /// The message language for all threads ("ru", "en"...). The default is the build
        /// language (ERRORS_LANG); an unknown code throws BerestaException(INVALID_ARGUMENT).
        /// </summary>
        public static string Language
        {
            get { return Marshal.PtrToStringUTF8(NativeMethods.beresta_language()) ?? string.Empty; }
            set { NativeCalls.Check(NativeMethods.beresta_set_language(value)); }
        }

        /// <summary>
        /// Все языки библиотеки.
        /// All languages of the library.
        /// </summary>
        public static IReadOnlyList<string> Languages
        {
            get
            {
                string joined = Marshal.PtrToStringUTF8(NativeMethods.beresta_languages()) ?? string.Empty;

                return joined.Split(',');
            }
        }

        /// <summary>
        /// Сообщение обёртки на текущем языке из каталога библиотеки (beresta_message).
        /// A binding message in the current language from the library catalog (beresta_message).
        /// </summary>
        internal static string Message(int message)
        {
            return Marshal.PtrToStringUTF8(NativeMethods.beresta_message_text(message)) ?? string.Empty;
        }
    }
}
