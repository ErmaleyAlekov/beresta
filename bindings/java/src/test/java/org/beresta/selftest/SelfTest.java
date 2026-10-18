package org.beresta.selftest;

import org.beresta.Beresta;
import org.beresta.Languages;

/**
 * Самопроверка Java-обёртки. Запускается без Maven и без JUnit:
 * {@code make JAVA=1 java-test} или {@code ctest} (BERESTA_BUILD_JAVA=ON).
 * Код возврата 0 — все проверки прошли, 1 — есть упавшие.
 *
 * <p>The Java wrapper self-test. Runs without Maven and without JUnit:
 * {@code make JAVA=1 java-test} or {@code ctest} (BERESTA_BUILD_JAVA=ON).
 * Exit code 0 means all checks passed, 1 means some failed.
 */
public final class SelfTest
{
    private static int failures;

    private SelfTest()
    {
    }

    /** Запуск одной проверки с печатью результата. / Runs one check and prints the result. */
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
        // Тексты ошибок сверяются по-русски — независимо от языка сборки
        // Error texts are compared in Russian — regardless of the build language
        Languages.use("ru");
        System.out.println("Берёста " + Beresta.version() + " для Java, платформа " + Beresta.platform());
        run("разбор и навигация", Check.PARSE_AND_NAVIGATE);
        run("построение и вывод", Check.BUILD_AND_SERIALIZE);
        run("копирование и удаление", Check.COPY_AND_REMOVE);
        run("перенос между документами", Check.MOVE_BETWEEN_DOCUMENTS);
        run("перенос: закрытый новый документ", Check.MOVE_CLOSED_TARGET);
        run("перенос: запрещённые случаи", Check.MOVE_ERRORS);
        run("ошибки", Check.ERRORS);
        run("файлы и кодировки", Check.FILES_AND_ENCODINGS);
        run("XPath: значения", Check.XPATH_VALUES);
        run("XPath: выборка", Check.XPATH_SELECT);
        run("XML Schema", Check.SCHEMA);
        run("языки сообщений / message languages", Check.LANGUAGES);
        run("неизвестный язык / unknown language", Check.UNKNOWN_LANGUAGE);
        System.out.println("Итого упало: " + failures);
        System.exit(Math.min(failures, 1));
    }
}
