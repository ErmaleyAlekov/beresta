package bench;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;

/**
 * Замер обёртки Java против стандартного стека XML из JDK на общих данных
 * (benchmarks/wrappers/data, см. make_data.js). Каждая операция — медиана
 * из RUNS прогонов после WARMUP прогревочных (JIT успевает скомпилировать
 * горячий код). Результат — строки таблицы Markdown.
 *   java --enable-native-access=ALL-UNNAMED -cp out bench.WrapperBench data
 *
 * A measurement of the Java wrapper against the standard JDK XML stack on common data
 * (benchmarks/wrappers/data, see make_data.js). Every operation is the median
 * of RUNS runs after WARMUP warm-up ones (the JIT has time to compile the
 * hot code). The result is Markdown table rows.
 */
public final class WrapperBench
{
    private static final int WARMUP = 3;
    private static final int RUNS = 5;

    /** Точка входа: путь к данным — первый аргумент. / The entry point: the data path is the first argument. */
    public static void main(String[] args) throws Exception
    {
        try
        {
            run(args);
        }
        catch (IOException failure)
        {
            System.err.println(BenchValues.READ_FAILED + failure.getMessage());
            System.exit(2);
        }
    }

    /** Читает данные и печатает результаты. / Reads input and prints results. */
    private static void run(String[] args) throws Exception
    {
        Path data = Path.of("data");
        if (args.length > 0)
            data = Path.of(args[0]);
        byte[] xml = Files.readAllBytes(data.resolve("catalog.xml"));
        String xsd = Files.readString(data.resolve("catalog.xsd"));
        String query = Files.readString(data.resolve("query.txt"));
        Engine[] engines = { new BerestaEngine(xsd, query), new JdkEngine(xsd, query) };

        // Режим памяти: --memory N меряет одну библиотеку в этом процессе
        // The memory mode: --memory N measures one library in this process
        if (args.length > 1 && args[1].equals("--memory"))
        {
            int index = BenchValues.memoryIndex(args, engines.length);
            if (index < 0)
            {
                System.err.println(BenchValues.MEMORY_INDEX_ERROR);
                System.exit(2);
                return;
            }
            System.out.println(MemoryProbe.measure(engines[index], xml));
            return;
        }
        System.out.println("### Java " + System.getProperty("java.version") + ", " + (xml.length / 1_000_000) + " MB\n");
        System.out.println("| Библиотека / Library | Разбор / Parse, MB/s | Обход / Walk, ms | XPath, ms | Разбор + XSD / Parse + XSD, MB/s | Элементы / Elements | XPath: узлы / nodes | Ошибки XSD / XSD errors | XSD valid |");
        System.out.println("|---|---|---|---|---|---|---|---|---|");
        for (Engine engine : engines)
            System.out.println(row(engine, xml));
    }

    /** Строка таблицы одной библиотеки. / The table row of one library. */
    private static String row(Engine engine, byte[] xml) throws Exception
    {
        double megabytes = xml.length / 1e6;
        double parse = median(engine, Operation.PARSE, xml, null);
        Object document = engine.parse(xml);
        try
        {
            double walk = median(engine, Operation.WALK, xml, document);
            double xpath = median(engine, Operation.XPATH, xml, document);
            double validate = median(engine, Operation.VALIDATE, xml, null);
            Engine.Validated validated = engine.parseValidated(xml);
            BenchValues.closeDocument(validated.document());
            String counts = engine.walk(document) + " | " + engine.xpath(document) + " | " + validated.errors();

            return String.format("| %s | %s | %.1f | %.1f | %s | %s | %s |", engine.name(),
                BenchValues.rate(megabytes, parse), walk, xpath, BenchValues.rate(megabytes, validate), counts,
                BenchValues.validity(validated.errors()));
        }
        finally
        {
            BenchValues.closeDocument(document);
        }
    }

    /** Медиана времени операции в миллисекундах. / The median operation time in milliseconds. */
    private static double median(Engine engine, Operation operation, byte[] xml, Object document) throws Exception
    {
        double[] times = new double[RUNS];

        for (int run = -WARMUP; run < RUNS; ++run)
        {
            long start = System.nanoTime();
            Object result = run(engine, operation, xml, document);
            long elapsed = System.nanoTime() - start;
            if (operation == Operation.PARSE)
                BenchValues.closeDocument(result);
            if (result instanceof Engine.Validated validated)
                BenchValues.closeDocument(validated.document());
            if (run >= 0)
                times[run] = elapsed / 1e6;
        }
        Arrays.sort(times);
        return times[RUNS / 2];
    }

    /** Одна операция над библиотекой. / One operation on a library. */
    private static Object run(Engine engine, Operation operation, byte[] xml, Object document) throws Exception
    {
        if (operation == Operation.PARSE)
            return engine.parse(xml);
        if (operation == Operation.WALK)
            return engine.walk(document);
        if (operation == Operation.XPATH)
            return engine.xpath(document);
        return engine.parseValidated(xml);
    }

}
