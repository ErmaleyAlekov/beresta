'use strict';

const { native } = require('../internal/native.js');
const { applyMixins } = require('../internal/mixin.js');
const { XmlNode } = require('./node.js');
const { ElementAttributes } = require('./element_attributes.js');
const { registerNodeClasses } = require('./wrapping.js');

/**
 * Элемент XML: узел с именем, атрибутами и пространством имён.
 * An XML element: a node with a name, attributes and a namespace.
 */
class XmlElement extends XmlNode
{
    /** Имя без префикса: у <s:item> — "item". / The name without a prefix: "item" for <s:item>. */
    get localName()
    {
        return native.element_local_name(this.handle);
    }

    /** URI пространства имён или null, если элемент вне пространств имён.
     *  The namespace URI or null if the element is in no namespace. */
    get namespaceUri()
    {
        return native.element_namespace_uri(this.handle);
    }

    /**
     * Снимок всех атрибутов { имя: значение } в порядке документа. Свойства
     * задаются через defineProperty: имя вроде "__proto__" не изменит прототип.
     *
     * A snapshot of all attributes { name: value } in document order. The properties
     * are set through defineProperty: a name like "__proto__" will not change the prototype.
     */
    get attributes()
    {
        const handle = this.handle;
        const count = native.element_attribute_count(handle);
        const result = {};

        for (let index = 0; index < count; ++index)
        {
            Object.defineProperty(result, native.element_attribute_name(handle, index), {
                value: native.element_attribute_value(handle, index),
                enumerable: true, writable: true, configurable: true
            });
        }
        return result;
    }
}

applyMixins(XmlElement, ElementAttributes);
registerNodeClasses(XmlNode, XmlElement);

module.exports = { XmlElement };
