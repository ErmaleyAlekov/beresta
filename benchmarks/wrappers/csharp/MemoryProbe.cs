using System;
using System.Diagnostics;
using System.Globalization;
using System.Threading;

namespace WrapperBench
{
    /// <summary>
    /// Память под дерево одной библиотеки: прирост рабочего набора процесса
    /// (WorkingSet64 — и управляемая куча, и нативная память «Берёсты») после
    /// разбора и сборки мусора. Каждая библиотека меряется в отдельном процессе.
    ///
    /// The tree memory of one library: the growth of the process working set
    /// (WorkingSet64 — both the managed heap and Beresta's native memory) after
    /// parsing and garbage collection. Every library is measured in a separate process.
    /// </summary>
    public static class MemoryProbe
    {
        /// <summary>Строка «имя | МБ | кратность к размеру файла». / A "name | MB | multiple of the file size" line.</summary>
        public static string Measure(Engine engine, byte[] xml)
        {
            Settle();
            long before = ResidentBytes();
            object document = engine.Parse(xml);
            Settle();
            long grown = ResidentBytes() - before;
            string line = string.Format(CultureInfo.InvariantCulture, "| {0} | {1} MB (×{2:F1}) |",
                engine.Name, grown / 1000000, (double)grown / xml.Length);

            engine.Walk(document);
            if (document is IDisposable disposable)
                disposable.Dispose();
            return line;
        }

        // Сборка мусора и короткая пауза, чтобы память улеглась / Garbage collection and a short pause so that the memory settles
        private static void Settle()
        {
            GC.Collect();
            GC.WaitForPendingFinalizers();
            Thread.Sleep(200);
            GC.Collect();
        }

        // Рабочий набор процесса в байтах / The process working set in bytes
        private static long ResidentBytes()
        {
            Process process = Process.GetCurrentProcess();
            process.Refresh();
            return process.WorkingSet64;
        }
    }
}
