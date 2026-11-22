package bench;

import java.util.Locale;

/** Проверка входных чисел и форматирование замеров. / Input number validation and measurement formatting. */
final class BenchValues
{
    static final String MEMORY_INDEX_ERROR = "--memory: нужен номер доступной библиотеки / expected an available library index";

    static final String READ_FAILED = "Не удалось прочитать входные данные / Could not read input: ";

    private BenchValues()
    {
    }

    /** Номер библиотеки или -1 при неверном аргументе. / The library index, or -1 for an invalid argument. */
    static int memoryIndex(String[] args, int engineCount)
    {
        if (args.length != 3 || !args[2].matches("[0-9]+"))
            return -1;
        try
        {
            int index = Integer.parseInt(args[2]);
            if (index < engineCount)
                return index;
        }
        catch (NumberFormatException outsideRange)
        {
            return -1;
        }
        return -1;
    }

    /** Скорость по измеряемому интервалу. / Throughput for a measurable interval. */
    static String rate(double megabytes, double milliseconds)
    {
        if (milliseconds <= 0)
            return "—";
        return String.format(Locale.ROOT, "%.0f", megabytes / milliseconds * 1000);
    }

    /** Сопоставимый итог проверки XSD. / Comparable XSD validation outcome. */
    static String validity(int errors)
    {
        if (errors == 0)
            return "yes";
        return "no";
    }

    /** Нативный документ закрывается явно. / A native document is explicitly closed. */
    static void closeDocument(Object document) throws Exception
    {
        if (document instanceof AutoCloseable closeable)
            closeable.close();
    }
}
