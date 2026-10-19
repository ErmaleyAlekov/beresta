using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Xml;
using Beresta;

namespace WrapperBench
{
    /// <summary>
    /// Потоковая проверка по XSD без дерева (dotnet run -- data --stream):
    /// «Берёста» (XmlSchema.Validate(bytes) — SAX + XSD внутри ядра) против
    /// проверяющего XmlReader, прочитанного до конца без построения
    /// XmlDocument/XDocument. Медиана из Runs прогонов после Warmup.
    ///
    /// Streaming XSD validation without a tree (dotnet run -- data --stream):
    /// Beresta (XmlSchema.Validate(bytes) — SAX + XSD inside the core) against
    /// a validating XmlReader read to the end without building an
    /// XmlDocument/XDocument. The median of Runs runs after Warmup.
    /// </summary>
    public static class StreamBench
    {
        private const int Warmup = 3;
        private const int Runs = 5;

        /// <summary>Таблица из двух строк. / A table of two rows.</summary>
        public static void Run(byte[] xml, string xsd)
        {
            double megabytes = xml.Length / 1e6;
            using Beresta.XmlSchema beresta = Beresta.XmlSchema.FromString(xsd);
            ErrorCounter reader = new ErrorCounter(xsd);

            Console.WriteLine("| Библиотека / Library | SAX + XSD, MB/s | Ошибки XSD / XSD errors |");
            Console.WriteLine("|---|---|---|");
            Console.WriteLine(string.Format(CultureInfo.InvariantCulture, "| Берёста / Beresta {0} (Validate(bytes)) | {1:F0} | {2} |",
                BerestaLibrary.Version, megabytes / Median(xml, beresta, null) * 1000, Once(xml, beresta, null)));
            Console.WriteLine(string.Format(CultureInfo.InvariantCulture, "| .NET {0} XmlReader (Schema) | {1:F0} | {2} |",
                Environment.Version, megabytes / Median(xml, null, reader) * 1000, Once(xml, null, reader)));
        }

        // Медиана времени в мс; задана одна из двух проверок / The median time in ms; one of the two validations is given
        private static double Median(byte[] xml, Beresta.XmlSchema? beresta, ErrorCounter? reader)
        {
            double[] times = new double[Runs];

            for (int run = -Warmup; run < Runs; ++run)
            {
                Stopwatch watch = Stopwatch.StartNew();
                Once(xml, beresta, reader);
                if (run >= 0)
                    times[run] = watch.Elapsed.TotalMilliseconds;
            }
            Array.Sort(times);
            return times[Runs / 2];
        }

        // Одна проверка; результат — число нарушений / One validation; the result is the violation count
        private static int Once(byte[] xml, Beresta.XmlSchema? beresta, ErrorCounter? reader)
        {
            if (beresta != null)
                return beresta.Validate(xml).Count;
            reader!.Errors = 0;
            using (XmlReader validating = XmlReader.Create(new MemoryStream(xml), reader.Settings))
            {
                while (validating.Read())
                {
                }
            }
            return reader.Errors;
        }
    }
}
