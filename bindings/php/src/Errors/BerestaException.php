<?php

declare(strict_types=1);

namespace Beresta;

use Beresta\Native\Library;
use Beresta\Native\Strings;

/**
 * Ошибка, о которой сообщила сама библиотека: синтаксис XML, XPath,
 * некорректная схема, ввод-вывод, нарушение структуры дерева и т. п.
 *
 * getCode() — число beresta_status, status() — то же как перечисление,
 * statusName() — имя вида "PARSE_ERROR", detail() — текст сообщения на
 * текущем языке библиотеки (Language::set).
 *
 * An error reported by the library itself: XML syntax, XPath, an invalid
 * schema, input-output, a tree structure violation and so on.
 *
 * getCode() is the beresta_status number, status() is the same as an enum,
 * statusName() is the name like "PARSE_ERROR", detail() is the message text in
 * the current library language (Language::set).
 */
class BerestaException extends \RuntimeException implements BerestaThrowable
{
    private readonly Status $status;
    private readonly string $statusName;
    private readonly string $detail;

    /**
     * Сообщение исключения — "[ИМЯ_КОДА] текст", как в других обёртках.
     * The exception message is "[CODE_NAME] text", as in the other bindings.
     */
    public function __construct(int $code, string $statusName, string $detail)
    {
        $this->status = Status::tryFrom($code) ?? Status::UnknownError;
        $this->statusName = $statusName;
        $this->detail = $detail;
        parent::__construct('[' . $statusName . '] ' . $detail, $code);
    }

    /**
     * Исключение по последней ошибке текущего потока. Сообщение читается
     * раньше имени кода: каждый вызов библиотеки сбрасывает последнюю ошибку.
     *
     * An exception for the last error of the current thread. The message is read
     * before the code name: every library call resets the last error.
     */
    public static function fromLastError(): self
    {
        $ffi = Library::ffi();
        $code = $ffi->beresta_last_error_code();
        $detail = Strings::borrowed($ffi->beresta_last_error_message());
        $name = Strings::borrowed($ffi->beresta_status_name($code));
        return new self($code, $name, $detail);
    }

    /**
     * Код состояния как перечисление. / The status code as an enum.
     */
    public function status(): Status
    {
        return $this->status;
    }

    /**
     * Имя кода, например "PARSE_ERROR". / The code name, for example "PARSE_ERROR".
     */
    public function statusName(): string
    {
        return $this->statusName;
    }

    /**
     * Текст сообщения без имени кода. / The message text without the code name.
     */
    public function detail(): string
    {
        return $this->detail;
    }
}
