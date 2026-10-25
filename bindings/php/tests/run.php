<?php

declare(strict_types=1);

/*
 * Самопроверка PHP-обёртки: php -d ffi.enable=1 tests/run.php
 * (или composer test, или make PHP=1 php-test в корне репозитория).
 *
 * The PHP binding self-test: php -d ffi.enable=1 tests/run.php
 * (or composer test, or make PHP=1 php-test in the repository root).
 */

require_once __DIR__ . '/../autoload.php';
require_once __DIR__ . '/Support/AssertionFailed.php';
require_once __DIR__ . '/Support/TestCase.php';
require_once __DIR__ . '/Support/TestRunner.php';

exit((new Beresta\Tests\Support\TestRunner())->run(__DIR__));
