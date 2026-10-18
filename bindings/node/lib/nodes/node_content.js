'use strict';

const { native } = require('../internal/native.js');
const { WriteFlags } = require('../flags.js');

/**
 * Примесь: тип, имя, текст и сериализация узла.
 * Mixin: the node type, name, text and serialization.
 */
class NodeContent
{
    /** Тип узла (значение NodeType). / The node type (a NodeType value). */
    get type()
    {
        return native.node_type(this.handle);
    }

    /** Имя по соглашению DOM: тег, цель PI, "#text", "#comment"...
     *  The name by the DOM convention: a tag, a PI target, "#text", "#comment"... */
    get name()
    {
        return native.node_name(this.handle);
    }

    /** Текстовое содержимое (у элемента — весь текст потомков).
     *  The text content (for an element — all text of its descendants). */
    get text()
    {
        return native.node_text(this.handle);
    }

    /** У элемента все дети заменяются одним текстовым узлом.
     *  For an element all children are replaced with one text node. */
    set text(value)
    {
        native.node_set_text(this.handle, String(value));
    }

    /** Сериализация узла с потомками; по умолчанию — без единого лишнего пробела.
     *  Serializes the node with its descendants; by default — without a single extra space. */
    toXml(flags = WriteFlags.DEFAULT)
    {
        return native.node_to_string(this.handle, flags);
    }
}

module.exports = { NodeContent };
