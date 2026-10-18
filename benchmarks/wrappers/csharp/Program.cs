using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;

namespace WrapperBench
{
    /// <summary>
    /// Замер обёртки .NET против System.Xml и LINQ to XML на общих данных
    /// (benchmarks/wrappers/data, см. make_data.js). Каждая операция — медиана
    /// из Runs прогонов после Warmup прогревочных (JIT и многоуровневая
    /// компиляция успевают отработать). Результат — строки таблицы Markdown.
    ///   dotnet run -c Release -- data
    ///
    /// A measurement of the .NET wrapper against System.Xml and LINQ to XML on common data
    /// (benchmarks/wrappers/data, see make_data.js). Every operation is the median
    /// of Runs runs after Warmup warm-up ones (the JIT and tiered compilation
    /// have time to work). The result is Markdown table rows.
    /// </summary>
    public static class Program
    {
        private const int Warmup = 3;
        private const int Runs = 5;

        private enum Operation { Parse, Walk, XPath, Validate }

        /// <summary>Точка входа: путь к данным — первый аргумент. / The entry point: the data path is the first argument.</summary>
        public static void Main(string[] args)
        {
            string data = "data";
            if (args.Length > 0)
                data = args[0];
            byte[] xml = File.ReadAllBytes(Path.Combine(data, "catalog.xml"));
            string xsd = File.ReadAllText(Path.Combine(data, "catalog.xsd"));
            string query = File.ReadAllText(Path.Combine(data, "query.txt"));
            Engine[] engines = { new BerestaEngine(xsd, query), new SystemXmlEngine(xsd, query), new LinqXmlEngine(xsd, query) };

            // Режим памяти: --memory N меряет одну библиотеку в этом процессе
            // The memory mode: --memory N measures one library in this process
            if (args.Length > 2 && args[1] == "--memory")
            {
                Console.WriteLine(MemoryProbe.Measure(engines[int.Parse(args[2], CultureInfo.InvariantCulture)], xml));
                return;
            }
            Console.WriteLine("### .NET " + Environment.Version + ", " + (xml.Length / 1000000) + " MB\n");
            Console.WriteLine("| Библиотека / Library | Разбор / Parse, MB/s | Обход / Walk, ms | XPath, ms | Разбор + XSD / Parse + XSD, MB/s | Элементы / Elements | XPath: узлы / nodes | Ошибки XSD / XSD errors |");
            Console.WriteLine("|---|---|---|---|---|---|---|---|");
            foreach (Engine engine in engines)
                Console.WriteLine(Row(engine, xml));
        }

        // Строка таблицы одной библиотеки / The table row of one library
        private static string Row(Engine engine, byte[] xml)
        {
            double megabytes = xml.Length / 1e6;
            double parse = Median(engine, Operation.Parse, xml, null);
            object document = engine.Parse(xml);
            double walk = Median(engine, Operation.Walk, xml, document);
            double xpath = Median(engine, Operation.XPath, xml, document);
            double validate = Median(engine, Operation.Validate, xml, null);
            string counts = engine.Walk(document) + " | " + engine.XPath(document) + " | " + engine.ParseValidated(xml);

            Release(document);
            return string.Format(System.Globalization.CultureInfo.InvariantCulture, "| {0} | {1:F0} | {2:F1} | {3:F1} | {4:F0} | {5} |",
                engine.Name, megabytes / parse * 1000, walk, xpath, megabytes / validate * 1000, counts);
        }

        // Медиана времени операции в миллисекундах / The median operation time in milliseconds
        private static double Median(Engine engine, Operation operation, byte[] xml, object? document)
        {
            double[] times = new double[Runs];

            for (int run = -Warmup; run < Runs; ++run)
            {
                Stopwatch watch = Stopwatch.StartNew();
                object result = Run(engine, operation, xml, document);
                watch.Stop();
                if (operation == Operation.Parse)
                    Release(result);
                if (run >= 0)
                    times[run] = watch.Elapsed.TotalMilliseconds;
            }
            Array.Sort(times);
            return times[Runs / 2];
        }

        // Одна операция над библиотекой / One operation on a library
        private static object Run(Engine engine, Operation operation, byte[] xml, object? document)
        {
            if (operation == Operation.Parse)
                return engine.Parse(xml);
            if (operation == Operation.Walk)
                return engine.Walk(document!);
            if (operation == Operation.XPath)
                return engine.XPath(document!);
            return engine.ParseValidated(xml);
        }

        // Документ «Берёсты» освобождается сразу, деревья .NET — сборщиком мусора
        // A Beresta document is freed at once, .NET trees — by the garbage collector
        private static void Release(object document)
        {
            if (document is IDisposable disposable)
                disposable.Dispose();
        }
    }
}
