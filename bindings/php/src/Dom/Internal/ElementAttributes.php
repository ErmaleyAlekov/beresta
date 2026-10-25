<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Native\Calls;
use Beresta\Native\Library;
use Beresta\Native\Strings;

/**
 * Трейт элемента: чтение, запись, проверка и удаление атрибутов.
 * An element trait: reading, writing, checking and removing attributes.
 */
trait ElementAttributes
{
    /**
     * Значение атрибута или $default, если атрибута нет.
     * The attribute value or $default if there is no such attribute.
     */
    public function getAttribute(string $name, ?string $default = null): ?string
    {
        return Strings::take(Library::ffi()->beresta_element_get_attribute($this->handle(), $name)) ?? $default;
    }

    /**
     * Запись атрибута (новый или замена значения). / Writes an attribute (a new one or a value replacement).
     */
    public function setAttribute(string $name, string $value): static
    {
        Calls::check(Library::ffi()->beresta_element_set_attribute($this->handle(), $name, $value));
        return $this;
    }

    /**
     * true — атрибут есть. / true means the attribute exists.
     */
    public function hasAttribute(string $name): bool
    {
        return Calls::checkFlag(Library::ffi()->beresta_element_has_attribute($this->handle(), $name));
    }

    /**
     * Удаление атрибута; true — он был, false — его не было.
     * Removes an attribute; true means it existed, false means it did not.
     */
    public function removeAttribute(string $name): bool
    {
        return Calls::checkFlag(Library::ffi()->beresta_element_remove_attribute($this->handle(), $name));
    }

    /**
     * Все атрибуты в порядке документа: имя => значение.
     * All attributes in document order: name => value.
     *
     * @return array<string, string>
     */
    public function attributes(): array
    {
        $ffi = Library::ffi();
        $handle = $this->handle();
        $count = Calls::checkInt($ffi->beresta_element_attribute_count($handle));
        $result = [];
        for ($index = 0; $index < $count; $index++) {
            $name = Strings::take($ffi->beresta_element_attribute_name($handle, $index)) ?? '';
            $result[$name] = Strings::take($ffi->beresta_element_attribute_value($handle, $index)) ?? '';
        }
        return $result;
    }
}
