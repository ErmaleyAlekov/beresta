using System;
using System.Runtime.InteropServices;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Ошибка «Берёсты». Code совпадает с beresta_status из Си-интерфейса,
    /// CodeName (например, PARSE_ERROR) одинаков во всех языках.
    /// </summary>
    public sealed class BerestaException : Exception
    {
        /// <summary>Создание по коду и тексту из Си-интерфейса.</summary>
        public BerestaException(int code, string detail)
            : base("[" + NameOf(code) + "] " + detail)
        {
            Code = code;
            CodeName = NameOf(code);
            Detail = detail;
        }

        /// <summary>Числовой код (beresta_status).</summary>
        public int Code { get; }

        /// <summary>Имя кода: PARSE_ERROR, VALIDATION_ERROR, INVALID_NAME...</summary>
        public string CodeName { get; }

        /// <summary>Текст ошибки без префикса с кодом.</summary>
        public string Detail { get; }

        private static string NameOf(int code)
        {
            return Marshal.PtrToStringUTF8(NativeMethods.beresta_status_name(code)) ?? "UNKNOWN_ERROR";
        }
    }
}
