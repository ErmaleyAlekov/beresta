<?php

declare(strict_types=1);

namespace Beresta\Tests\Support;

/**
 * Провал проверки в тесте: отличается от прочих исключений, чтобы прогонщик
 * писал «FAIL» (не выполнено ожидание), а не «ERROR» (тест упал сам).
 *
 * A failed check in a test: it differs from other exceptions so that the runner
 * prints "FAIL" (an expectation is not met), not "ERROR" (the test itself crashed).
 */
final class AssertionFailed extends \RuntimeException
{
}
