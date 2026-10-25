<?php

declare(strict_types=1);

namespace Beresta\Tests;

use Beresta\BerestaException;
use Beresta\BerestaThrowable;
use Beresta\ClosedException;
use Beresta\Document;
use Beresta\Language;
use Beresta\Parser;
use Beresta\Status;
use Beresta\Tests\Support\TestCase;

/**
 * Язык сообщений и вид исключений. Язык общий для процесса, поэтому тест,
 * который его меняет, возвращает прежний.
 *
 * The message language and the shape of exceptions. The language is common to the
 * process, so a test that changes it restores the former one.
 */
final class LanguageTest extends TestCase
{
    /**
     * Смена языка меняет и сообщения библиотеки, и сообщения обёртки.
     * Changing the language changes both the library and the binding messages.
     */
    public function testLanguageSwitch(): void
    {
        $this->assertTrue(in_array('en', Language::available(), true));
        $before = Language::current();
        Language::set('en');
        try {
            $this->assertSame('en', Language::current());
            $document = Document::create('a');
            $document->close();
            $error = $this->assertThrows(ClosedException::class, fn() => $document->root());
            $this->assertTrue(str_contains($error->getMessage(), 'closed'));
        } finally {
            Language::set($before);
        }
    }

    /**
     * Неизвестный язык — INVALID_ARGUMENT, текущий язык не меняется.
     * An unknown language is INVALID_ARGUMENT, the current language does not change.
     */
    public function testUnknownLanguage(): void
    {
        $before = Language::current();
        $error = $this->assertThrows(BerestaException::class, fn() => Language::set('xx'));
        $this->assertSame(Status::InvalidArgument, $error->status());
        $this->assertSame($before, Language::current());
    }

    /**
     * BerestaException: код, имя кода, текст и общий признак BerestaThrowable.
     * BerestaException: the code, the code name, the text and the common BerestaThrowable marker.
     */
    public function testExceptionShape(): void
    {
        $error = $this->assertThrows(BerestaException::class, fn() => (new Parser())->parseString('<a>'));
        $this->assertSame(Status::ParseError->value, $error->getCode());
        $this->assertSame('PARSE_ERROR', $error->statusName());
        $this->assertSame('[PARSE_ERROR] ' . $error->detail(), $error->getMessage());
        $this->assertTrue($error instanceof BerestaThrowable);
    }
}
