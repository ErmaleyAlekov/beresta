<?php

declare(strict_types=1);

namespace Beresta\Tests\Support;

/**
 * Основа тестового класса: несколько проверок без PHPUnit.
 *
 * Своя маленькая основа вместо PHPUnit — как unittest у Python-обёртки и
 * node --test у Node.js: тесты запускаются одной командой php без
 * composer install и без сторонних пакетов. Тест — публичный метод,
 * имя которого начинается с test.
 *
 * The base of a test class: a few checks without PHPUnit.
 *
 * An own small base instead of PHPUnit — like unittest for the Python binding and
 * node --test for Node.js: the tests run with one php command without composer
 * install and without third-party packages. A test is a public method whose name
 * starts with test.
 */
abstract class TestCase
{
    /**
     * Строгое равенство (===). / Strict equality (===).
     */
    protected function assertSame(mixed $expected, mixed $actual, string $message = ''): void
    {
        if ($expected !== $actual) {
            $this->fail('ожидалось / expected ' . var_export($expected, true)
                . ', получено / got ' . var_export($actual, true) . ' ' . $message);
        }
    }

    /**
     * Условие истинно. / The condition is true.
     */
    protected function assertTrue(bool $condition, string $message = ''): void
    {
        if (!$condition) {
            $this->fail('условие ложно / the condition is false ' . $message);
        }
    }

    /**
     * Значение — null. / The value is null.
     */
    protected function assertNull(mixed $actual, string $message = ''): void
    {
        $this->assertSame(null, $actual, $message);
    }

    /**
     * Действие бросает исключение класса $class; оно возвращается для
     * дальнейших проверок (код, текст).
     *
     * The action throws an exception of the $class class; it is returned for
     * further checks (the code, the text).
     *
     * @template T of \Throwable
     * @param class-string<T> $class
     * @return T
     */
    protected function assertThrows(string $class, callable $action): \Throwable
    {
        try {
            $action();
        } catch (\Throwable $error) {
            if ($error instanceof $class) {
                return $error;
            }
            $this->fail('ожидалось / expected ' . $class . ', получено / got ' . get_class($error)
                . ': ' . $error->getMessage());
        }
        $this->fail('исключение не брошено / no exception thrown: ' . $class);
    }

    /**
     * Провал теста с пояснением. / Fails the test with an explanation.
     */
    protected function fail(string $message): never
    {
        throw new AssertionFailed($message);
    }
}
