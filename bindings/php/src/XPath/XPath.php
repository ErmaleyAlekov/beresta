<?php

declare(strict_types=1);

namespace Beresta;

use Beresta\Internal\XPathBindings;
use Beresta\Internal\XPathEvaluation;
use Beresta\Native\Calls;
use Beresta\Native\Handles;
use Beresta\Native\Library;
use FFI\CData;

/**
 * Скомпилированное выражение XPath 1.0.
 *
 * Компилируется один раз, вычисляется сколько угодно раз для разных узлов
 * и документов. Префиксы и переменные задаются в конструкторе или позже.
 *
 *     $byCode = new XPath('//s:item[@code = $c]', ['s' => 'urn:shop'], ['c' => 'B2']);
 *     $items = $byCode->select($document);             // list<Node|string>
 *     $prices = (new XPath('//item/@price'))->selectValues($document);
 *     $total = (new XPath('sum(//item/@price)'))->number($document);
 *
 * A compiled XPath 1.0 expression.
 *
 * It is compiled once and evaluated any number of times for different nodes and
 * documents. Prefixes and variables are set in the constructor or later.
 */
final class XPath
{
    use XPathBindings;
    use XPathEvaluation;

    /**
     * Дескриптор выражения; освобождается деструктором.
     * The expression handle; freed by the destructor.
     */
    private readonly CData $pointer;

    /**
     * @param array<string, string> $namespaces префикс => URI / prefix => URI
     * @param array<string, string|int|float|\Stringable> $variables имя => значение / name => value
     */
    public function __construct(string $expression, array $namespaces = [], array $variables = [])
    {
        $slot = Handles::slot();
        Calls::check(Library::ffi()->beresta_xpath_compile($expression, Handles::out($slot)));
        $this->pointer = $slot;
        // Если конструктор бросит исключение, PHP не вызовет деструктор —
        // поэтому при ошибке настройки выражение освобождается здесь
        // If the constructor throws, PHP does not call the destructor —
        // so on a configuration error the expression is freed here
        try {
            $this->configure($namespaces, $variables);
        } catch (\Throwable $error) {
            Library::ffi()->beresta_xpath_free($slot);
            throw $error;
        }
    }

    /**
     * Освобождение скомпилированного выражения. / Frees the compiled expression.
     */
    public function __destruct()
    {
        Library::ffi()->beresta_xpath_free($this->pointer);
    }

    /**
     * Дескриптор для вызова Си-функций. / The handle for calling C functions.
     *
     * @internal
     */
    public function handle(): CData
    {
        return $this->pointer;
    }

    /**
     * Префиксы и переменные из конструктора. / The prefixes and variables from the constructor.
     *
     * @param array<string, string> $namespaces
     * @param array<string, string|int|float|\Stringable> $variables
     */
    private function configure(array $namespaces, array $variables): void
    {
        foreach ($namespaces as $prefix => $uri) {
            $this->bind((string) $prefix, $uri);
        }
        foreach ($variables as $name => $value) {
            $this->setVariable((string) $name, $value);
        }
    }
}
