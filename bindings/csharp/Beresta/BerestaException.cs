using System;
using System.Runtime.InteropServices;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Ошибка «Берёсты». Code совпадает с beresta_status из Си-интерфейса,
    /// CodeName (например, PARSE_ERROR) одинаков во всех языках.
    /// A Beresta error. Code matches beresta_status of the C interface,
    /// CodeName (for example, PARSE_ERROR) is the same in all languages.
    /// </summary>
    public sealed class BerestaException : Exception
    {
        /// <summary>Создание по коду и тексту из Си-интерфейса.
        /// Created from the code and the text of the C interface.</summary>
        public BerestaException(int code, string detail)
            : base("[" + NameOf(code) + "] " + detail)
        {
            Code = code;
            CodeName = NameOf(code);
            Detail = detail;
        }

        /// <summary>Числовой код (beresta_status).
        /// The numeric code (beresta_status).</summary>
        public int Code { get; }

        /// <summary>Имя кода: PARSE_ERROR, VALIDATION_ERROR, INVALID_NAME...
        /// The code name: PARSE_ERROR, VALIDATION_ERROR, INVALID_NAME...</summary>
        public string CodeName { get; }

        /// <summary>Текст ошибки без префикса с кодом.
        /// The error text without the code prefix.</summary>
        public string Detail { get; }

        private static string NameOf(int code)
        {
            return Marshal.PtrToStringUTF8(NativeMethods.beresta_status_name(code)) ?? "UNKNOWN_ERROR";
        }
    }
}
