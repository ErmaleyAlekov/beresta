package bench;

import java.nio.file.Files;
import java.nio.file.Path;

/**
 * Память под дерево одной библиотеки: прирост резидентной памяти процесса
 * (VmRSS из /proc/self/status, Linux) после разбора и сборки мусора. Её
 * видно и для кучи Java, и для нативной памяти «Берёсты». Каждая библиотека
 * меряется в отдельном процессе, чтобы не мешали остатки предыдущей.
 *
 * The tree memory of one library: the growth of the process resident memory
 * (VmRSS from /proc/self/status, Linux) after parsing and garbage collection. It
 * covers both the Java heap and Beresta's native memory. Every library
 * is measured in a separate process so that leftovers of the previous one do not interfere.
 */
public final class MemoryProbe
{
    private MemoryProbe()
    {
    }

    /** Строка «имя | МБ | кратность к размеру файла». / A "name | MB | multiple of the file size" line. */
    public static String measure(Engine engine, byte[] xml) throws Exception
    {
        settle();
        long before = residentBytes();
        Object document = engine.parse(xml);
        settle();
        long grown = residentBytes() - before;
        String line = String.format("| %s | %d MB (×%.1f) |", engine.name(), grown / 1_000_000, (double) grown / xml.length);

        engine.walk(document);
        if (document instanceof AutoCloseable closeable)
            closeable.close();
        return line;
    }

    /** Сборка мусора и короткая пауза, чтобы память улеглась. / Garbage collection and a short pause so that the memory settles. */
    private static void settle() throws InterruptedException
    {
        System.gc();
        Thread.sleep(200);
        System.gc();
    }

    /** VmRSS процесса в байтах. / The process VmRSS in bytes. */
    private static long residentBytes() throws Exception
    {
        for (String line : Files.readAllLines(Path.of("/proc/self/status")))
        {
            if (line.startsWith("VmRSS:"))
                return Long.parseLong(line.replaceAll("[^0-9]", "")) * 1024;
        }
        return 0;
    }
}
