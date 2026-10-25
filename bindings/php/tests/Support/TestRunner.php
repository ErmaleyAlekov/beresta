<?php

declare(strict_types=1);

namespace Beresta\Tests\Support;

/**
 * Прогонщик тестов: находит tests/*Test.php, выполняет методы test* и
 * печатает итог. Каждый тест — в новом экземпляре класса, чтобы состояние
 * одного теста не влияло на другой.
 *
 * The test runner: finds tests/*Test.php, runs the test* methods and prints the
 * summary. Every test runs in a new class instance so that the state of one test
 * does not affect another.
 */
final class TestRunner
{
    private int $passed = 0;

    /** @var list<string> */
    private array $failures = [];

    /**
     * Все файлы *Test.php каталога; код возврата 0 — все тесты прошли.
     * All *Test.php files of the directory; exit code 0 means all tests passed.
     */
    public function run(string $directory): int
    {
        $files = glob($directory . '/*Test.php');
        sort($files);
        foreach ($files as $file) {
            require_once $file;
            $this->runClass('Beresta\\Tests\\' . basename($file, '.php'));
        }
        return $this->report();
    }

    /**
     * Все методы test* одного класса. / All test* methods of one class.
     *
     * @param class-string<TestCase> $class
     */
    private function runClass(string $class): void
    {
        foreach (get_class_methods($class) as $method) {
            if (str_starts_with($method, 'test')) {
                $this->runTest($class, $method);
            }
        }
    }

    /**
     * Один тест: FAIL — не выполнено ожидание, ERROR — непредвиденное исключение.
     * One test: FAIL means an expectation is not met, ERROR means an unexpected exception.
     *
     * @param class-string<TestCase> $class
     */
    private function runTest(string $class, string $method): void
    {
        $name = substr(strrchr($class, '\\'), 1) . '::' . $method;
        try {
            (new $class())->$method();
            $this->passed++;
            echo 'ok    ', $name, PHP_EOL;
        } catch (AssertionFailed $failure) {
            $this->failures[] = 'FAIL  ' . $name . ': ' . $failure->getMessage();
        } catch (\Throwable $error) {
            $this->failures[] = 'ERROR ' . $name . ': ' . get_class($error) . ': ' . $error->getMessage()
                . ' (' . $error->getFile() . ':' . $error->getLine() . ')';
        }
    }

    /**
     * Итог: провалы и строка «пройдено N из M». / The summary: failures and the "passed N of M" line.
     */
    private function report(): int
    {
        foreach ($this->failures as $failure) {
            echo $failure, PHP_EOL;
        }
        $total = $this->passed + count($this->failures);
        echo PHP_EOL, 'Пройдено / Passed: ', $this->passed, '/', $total, PHP_EOL;
        if ($this->failures === [] && $total > 0) {
            return 0;
        }
        return 1;
    }
}
