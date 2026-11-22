<?php

declare(strict_types=1);

namespace Beresta\Tests;

use Beresta\Tests\Support\AssertionFailed;
use Beresta\Tests\Support\TestCase;

/**
 * Проверка диагностики утверждений.
 * Assertion diagnostics test.
 */
final class AssertionDiagnosticsTest extends TestCase
{
    /**
     * Проверка, что исключение сохраняет причину при неожиданном исключении.
     * Checks that the exception retains the cause with an unexpected exception.
     */
    public function testUnexpectedExceptionRetainsCause(): void
    {
        $runtimeException = new \RuntimeException('Unexpected error');
        $error = $this->assertThrows(AssertionFailed::class,
            fn() => $this->assertThrows(\LogicException::class, fn() => throw $runtimeException));
        $this->assertSame($runtimeException, $error->getPrevious());
    }
}
