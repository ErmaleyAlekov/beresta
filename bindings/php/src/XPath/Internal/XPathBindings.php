<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Native\Calls;
use Beresta\Native\Library;

/**
 * Трейт XPath: префиксы пространств имён и переменные $name.
 * An XPath trait: namespace prefixes and $name variables.
 */
trait XPathBindings
{
    /**
     * Префикс для имён вида p:name в выражении. / A prefix for p:name names in the expression.
     */
    public function bind(string $prefix, string $uri): static
    {
        Calls::check(Library::ffi()->beresta_xpath_bind_namespace($this->handle(), $prefix, $uri));
        return $this;
    }

    /**
     * Переменная $name: число (int, float) или строка (string, \Stringable).
     * The $name variable: a number (int, float) or a string (string, \Stringable).
     */
    public function setVariable(string $name, string|int|float|\Stringable $value): static
    {
        $ffi = Library::ffi();
        if (is_int($value) || is_float($value)) {
            Calls::check($ffi->beresta_xpath_set_number($this->handle(), $name, (float) $value));
            return $this;
        }
        Calls::check($ffi->beresta_xpath_set_string($this->handle(), $name, (string) $value));
        return $this;
    }
}
