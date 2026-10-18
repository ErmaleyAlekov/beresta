using System;
using System.Collections.Generic;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Одно нарушение схемы: текст начинается с пути к элементу
    /// ("/catalog/book[2]: ..."), Line и Column — место в исходнике (0 — неизвестно).
    /// One schema violation: the text starts with the element path
    /// ("/catalog/book[2]: ..."), Line and Column are the place in the source (0 — unknown).
    /// </summary>
    public sealed class Diagnostic
    {
        internal Diagnostic(string message, long line, long column)
        {
            Message = message;
            Line = line;
            Column = column;
        }

        /// <summary>Текст нарушения.
        /// The violation text.</summary>
        public string Message { get; }

        /// <summary>Строка в исходнике.
        /// The line in the source.</summary>
        public long Line { get; }

        /// <summary>Столбец в исходнике.
        /// The column in the source.</summary>
        public long Column { get; }

        /// <summary>Чтение нативного списка с его освобождением в finally.
        /// Reads the native list and releases it in finally.</summary>
        internal static List<Diagnostic> ReadAll(IntPtr list)
        {
            List<Diagnostic> result = new List<Diagnostic>();

            try
            {
                long count = NativeMethods.beresta_diagnostics_count(list);
                for (long index = 0; index < count; ++index)
                    result.Add(new Diagnostic(
                        NativeCalls.TakeRequiredString(NativeMethods.beresta_diagnostics_message(list, index)),
                        NativeMethods.beresta_diagnostics_line(list, index),
                        NativeMethods.beresta_diagnostics_column(list, index)));
                return result;
            }
            finally
            {
                NativeMethods.beresta_diagnostics_free(list);
            }
        }
    }
}
