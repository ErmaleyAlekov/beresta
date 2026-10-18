'use strict';

const { native } = require('../internal/native.js');

/**
 * Примесь XPath: контекст выражения — префиксы пространств имён и
 * переменные $name. Методы возвращают само выражение — удобно цепочкой.
 *
 * An XPath mixin: the expression context — namespace prefixes and $name
 * variables. The methods return the expression itself — handy for chaining.
 */
class XPathContext
{
    /** Префикс для имён p:name в выражении. / A prefix for p:name names in the expression. */
    bindNamespace(prefix, uri)
    {
        native.xpath_bind_namespace(this.handle, prefix, uri);
        return this;
    }

    /** Переменная $name: число остаётся числом, остальное становится строкой.
     *  The $name variable: a number stays a number, everything else becomes a string. */
    set(name, value)
    {
        if (typeof value === 'number')
            native.xpath_set_number(this.handle, name, value);
        else
            native.xpath_set_string(this.handle, name, String(value));
        return this;
    }
}

module.exports = { XPathContext };
