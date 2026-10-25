<?php

declare(strict_types=1);

namespace Beresta\Native;

use Beresta\BerestaException;
use FFI\CData;

/**
 * Проверка результатов Си-функций и превращение сбоев в исключения.
 *
 * Си-интерфейс сообщает об ошибке кодом состояния или особым значением
 * результата (NULL, -1) и «последней ошибкой потока». Здесь собраны все
 * виды таких проверок, чтобы классы обёртки не повторяли их сами.
 *
 * Checking the results of C functions and turning failures into exceptions.
 *
 * The C interface reports an error with a status code or a special result value
 * (NULL, -1) and the "last error of the thread". All kinds of such checks are
 * gathered here so that the wrapper classes do not repeat them.
 */
final class Calls
{
    /**
     * int-статус: всё, кроме BERESTA_OK (0), — исключение.
     * An int status: anything except BERESTA_OK (0) is an exception.
     */
    public static function check(int $status): void
    {
        if ($status !== 0) {
            throw BerestaException::fromLastError();
        }
    }

    /**
     * Дескриптор-результат: NULL без ошибки — законный ответ «объекта нет».
     * A handle result: NULL without an error is a legitimate answer "there is no object".
     */
    public static function checkHandle(?CData $pointer): ?CData
    {
        if ($pointer === null && Library::ffi()->beresta_last_error_code() !== 0) {
            throw BerestaException::fromLastError();
        }
        return $pointer;
    }

    /**
     * Целое (размер, индекс, тип узла), где -1 означает ошибку.
     * An integer (a size, an index, a node type) where -1 means an error.
     */
    public static function checkInt(int $value): int
    {
        if ($value < 0) {
            throw BerestaException::fromLastError();
        }
        return $value;
    }

    /**
     * Ответ «да / нет»: 1 — да, 0 — нет, -1 — ошибка.
     * A "yes / no" answer: 1 is yes, 0 is no, -1 is an error.
     */
    public static function checkFlag(int $value): bool
    {
        return self::checkInt($value) === 1;
    }
}
