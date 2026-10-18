'use strict';

const { native } = require('../internal/native.js');
const { wrap } = require('./wrapping.js');

/**
 * Примесь: добавление новых узлов в конец детей. Каждый метод возвращает
 * созданный узел — удобно строить дерево цепочкой:
 * root.appendElement('item').appendText('Чай').
 *
 * Mixin: appending new nodes to the end of the children. Each method returns
 * the created node — convenient for building a tree in a chain:
 * root.appendElement('item').appendText('Чай').
 */
class NodeEditing
{
    /** Новый элемент; у пустого документа он становится корнем. / A new element; in an empty document it becomes the root. */
    appendElement(name)
    {
        return wrap(this.document, native.node_append_element(this.handle, name));
    }

    /** Текстовый узел (спецсимволы экранируются при выводе). / A text node (special characters are escaped on output). */
    appendText(text)
    {
        return wrap(this.document, native.node_append_text(this.handle, text));
    }

    /** Комментарий <!--text-->. / A comment <!--text-->. */
    appendComment(text)
    {
        return wrap(this.document, native.node_append_comment(this.handle, text));
    }

    /** Секция <![CDATA[text]]>. / A <![CDATA[text]]> section. */
    appendCData(text)
    {
        return wrap(this.document, native.node_append_cdata(this.handle, text));
    }

    /** Инструкция обработки <?target data?>; data необязательна. / A processing instruction <?target data?>; data is optional. */
    appendProcessingInstruction(target, data = null)
    {
        return wrap(this.document, native.node_append_pi(this.handle, target, data));
    }
}

module.exports = { NodeEditing };
