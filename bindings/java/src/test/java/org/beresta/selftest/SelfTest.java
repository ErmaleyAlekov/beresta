package org.beresta.selftest;

import org.beresta.Beresta;

/**
 * Самопроверка Java-обёртки. Запускается без Maven и без JUnit:
 * {@code make JAVA=1 java-test} или {@code ctest} (BERESTA_BUILD_JAVA=ON).
 * Код возврата 0 — все проверки прошли, 1 — есть упавшие.
 */
public final class SelfTest
{
    private static int failures;

    private SelfTest()
    {
    }

    /** Запуск одной проверки с печатью результата. */
    private static void run(String name, Check check)
    {
        try
        {
            check.run();
            System.out.println("[ OK ] " + name);
        }
        catch (Throwable error)
        {
            ++failures;
            System.out.println("[FAIL] " + name + ": " + error);
        }
    }

    public static void main(String[] arguments)
    {
        System.out.println("Берёста " + Beresta.version() + " для Java, платформа " + Beresta.platform());
        run("разбор и навигация", Check.PARSE_AND_NAVIGATE);
        run("построение и вывод", Check.BUILD_AND_SERIALIZE);
        run("копирование и удаление", Check.COPY_AND_REMOVE);
        run("ошибки", Check.ERRORS);
        run("файлы и кодировки", Check.FILES_AND_ENCODINGS);
        run("XPath: значения", Check.XPATH_VALUES);
        run("XPath: выборка", Check.XPATH_SELECT);
        run("XML Schema", Check.SCHEMA);
        System.out.println("Итого упало: " + failures);
        System.exit(Math.min(failures, 1));
    }
}
