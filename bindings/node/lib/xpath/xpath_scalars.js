'use strict';

const { native } = require('../internal/native.js');

/**
 * Примесь XPath: результат как строка, число или логическое значение —
 * по правилам функций string(), number(), boolean() XPath 1.0.
 *
 * An XPath mixin: the result as a string, a number or a boolean —
 * by the rules of the XPath 1.0 string(), number(), boolean() functions.
 */
class XPathScalars
{
    /** Результат по правилам функции string(). / The result by the rules of the string() function. */
    string(context)
    {
        return native.xpath_string(this.handle, context.handle);
    }

    /** Результат по правилам функции number(); NaN — не ошибка, а законный ответ.
     *  The result by the rules of the number() function; NaN is not an error but a legitimate answer. */
    number(context)
    {
        return native.xpath_number(this.handle, context.handle);
    }

    /** Результат по правилам функции boolean(). / The result by the rules of the boolean() function. */
    boolean(context)
    {
        return native.xpath_boolean(this.handle, context.handle);
    }
}

module.exports = { XPathScalars };
