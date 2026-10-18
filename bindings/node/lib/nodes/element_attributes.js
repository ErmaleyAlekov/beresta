'use strict';

const { native } = require('../internal/native.js');

/**
 * Примесь элемента: чтение и запись атрибутов по имени.
 * An element mixin: reading and writing attributes by name.
 */
class ElementAttributes
{
    /** Значение атрибута или null, если атрибута нет. / The attribute value or null if there is no such attribute. */
    getAttribute(name)
    {
        return native.element_get_attribute(this.handle, name);
    }

    /** Установка или замена значения; недопустимое имя — BerestaError(INVALID_NAME).
     *  Sets or replaces the value; an invalid name is BerestaError(INVALID_NAME). */
    setAttribute(name, value)
    {
        native.element_set_attribute(this.handle, name, String(value));
        return this;
    }

    /** Есть ли атрибут с таким именем. / Whether there is an attribute with this name. */
    hasAttribute(name)
    {
        return native.element_has_attribute(this.handle, name);
    }

    /** true — атрибут был и удалён, false — его не было. / true means it existed and was removed, false means it did not exist. */
    removeAttribute(name)
    {
        return native.element_remove_attribute(this.handle, name);
    }
}

module.exports = { ElementAttributes };
