<?php

declare(strict_types=1);

namespace Beresta;

use Beresta\Internal\Message;

/**
 * Обращение к узлу документа, который уже закрыт (Document::close).
 *
 * Это ошибка программы, а не данных, поэтому исключение логическое. Без
 * этой проверки вызов ушёл бы в освобождённую память и уронил бы процесс.
 * Текст — из каталога сообщений библиотеки на текущем языке.
 *
 * Access to a node of a document that is already closed (Document::close).
 *
 * This is a program error, not a data error, so the exception is a logic one.
 * Without this check the call would go into freed memory and crash the process.
 * The text comes from the library message catalog in the current language.
 */
final class ClosedException extends \LogicException implements BerestaThrowable
{
    /**
     * Документ закрыт. / The document is closed.
     */
    public static function document(): self
    {
        return new self('Берёста / Beresta: ' . Language::message(Message::DocumentClosed));
    }
}
