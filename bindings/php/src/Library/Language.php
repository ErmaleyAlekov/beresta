<?php

declare(strict_types=1);

namespace Beresta;

use Beresta\Internal\Message;
use Beresta\Native\Calls;
use Beresta\Native\Library;
use Beresta\Native\Strings;

/**
 * Язык сообщений об ошибках: библиотеки и самой обёртки.
 *
 *     Language::set('en');
 *     echo Language::current();          // en
 *     print_r(Language::available());    // ['ru', 'en']
 *
 * Язык общий для всех потоков процесса.
 *
 * The language of error messages: of the library and of the binding itself.
 * The language is common to all threads of the process.
 */
final class Language
{
    /**
     * Смена языка; неизвестный код — BerestaException INVALID_ARGUMENT.
     * Changes the language; an unknown code is a BerestaException INVALID_ARGUMENT.
     */
    public static function set(string $code): void
    {
        Calls::check(Library::ffi()->beresta_set_language($code));
    }

    /**
     * Код текущего языка, например "ru". / The current language code, for example "ru".
     */
    public static function current(): string
    {
        return Strings::borrowed(Library::ffi()->beresta_language());
    }

    /**
     * Все языки каталога. / All catalog languages.
     *
     * @return list<string>
     */
    public static function available(): array
    {
        return explode(',', Strings::borrowed(Library::ffi()->beresta_languages()));
    }

    /**
     * Текст сообщения обёртки; {0}, {1}... заменяются аргументами по порядку.
     * The binding message text; {0}, {1}... are replaced with the arguments in order.
     *
     * @internal
     */
    public static function message(Message $message, string ...$arguments): string
    {
        $text = Strings::borrowed(Library::ffi()->beresta_message_text($message->value));
        foreach ($arguments as $index => $argument) {
            $text = str_replace('{' . $index . '}', $argument, $text);
        }
        return $text;
    }
}
