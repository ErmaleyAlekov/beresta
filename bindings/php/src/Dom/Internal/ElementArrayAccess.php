<?php

declare(strict_types=1);

namespace Beresta\Internal;

use Beresta\Language;

/**
 * Трейт элемента: атрибуты как элементы массива (\ArrayAccess), как в SimpleXML.
 * Ключ — имя атрибута; значение записи — строка, число или \Stringable.
 *
 * An element trait: attributes as array elements (\ArrayAccess), as in SimpleXML.
 * The key is the attribute name; the written value is a string, a number or a \Stringable.
 */
trait ElementArrayAccess
{
    /**
     * isset($element['name']). / isset($element['name']).
     */
    public function offsetExists(mixed $offset): bool
    {
        return $this->hasAttribute(self::attributeName($offset, 'offsetExists'));
    }

    /**
     * $element['name'] — значение или null. / $element['name'] is the value or null.
     */
    public function offsetGet(mixed $offset): ?string
    {
        return $this->getAttribute(self::attributeName($offset, 'offsetGet'));
    }

    /**
     * $element['name'] = $value. Запись без ключа ($element[] = ...) — ошибка.
     * $element['name'] = $value. Writing without a key ($element[] = ...) is an error.
     */
    public function offsetSet(mixed $offset, mixed $value): void
    {
        $name = self::attributeName($offset, 'offsetSet');
        if (!is_scalar($value) && !$value instanceof \Stringable) {
            throw new \InvalidArgumentException(Language::message(Message::InvalidArgument, '2', 'offsetSet'));
        }
        $this->setAttribute($name, (string) $value);
    }

    /**
     * unset($element['name']). / unset($element['name']).
     */
    public function offsetUnset(mixed $offset): void
    {
        $this->removeAttribute(self::attributeName($offset, 'offsetUnset'));
    }

    /**
     * Ключ массива как имя атрибута: только непустая строка.
     * The array key as an attribute name: only a non-empty string.
     */
    private static function attributeName(mixed $offset, string $method): string
    {
        if (!is_string($offset) || $offset === '') {
            throw new \InvalidArgumentException(Language::message(Message::InvalidArgument, '1', $method));
        }
        return $offset;
    }
}
