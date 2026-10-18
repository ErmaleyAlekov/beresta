'use strict';

const { native } = require('../internal/native.js');
const { HANDLE } = require('../internal/keys.js');
const { track, release } = require('../internal/release.js');
const { applyMixins } = require('../internal/mixin.js');
const { BerestaClosedError } = require('../errors.js');
const { messageText, Messages } = require('../languages.js');
const { XPathContext } = require('./xpath_context.js');
const { XPathEvaluation } = require('./xpath_evaluation.js');
const { XPathScalars } = require('./xpath_scalars.js');

/**
 * Скомпилированное выражение XPath 1.0. Компилируется один раз, вычисляется
 * сколько угодно раз для разных узлов и документов. Префиксы и переменные
 * задаются в конструкторе или позже (bindNamespace, set).
 *   new XPath('//s:item[@code = $c]', { namespaces: { s: 'urn:shop' }, variables: { c: 'B2' } })
 *
 * A compiled XPath 1.0 expression. It is compiled once and evaluated any number
 * of times for different nodes and documents. Prefixes and variables are set
 * in the constructor or later (bindNamespace, set).
 */
class XPath
{
    constructor(expression, options = {})
    {
        this[HANDLE] = native.xpath_compile(expression);
        track(this, 'xpath_free', this[HANDLE]);
        for (const [prefix, uri] of Object.entries(options.namespaces ?? {}))
            this.bindNamespace(prefix, uri);
        for (const [name, value] of Object.entries(options.variables ?? {}))
            this.set(name, value);
    }

    /** Дескриптор; BerestaClosedError, если выражение уже закрыто. / The handle; BerestaClosedError if the expression is closed. */
    get handle()
    {
        if (this[HANDLE] === null)
            throw new BerestaClosedError(messageText(Messages.XPATH_CLOSED));
        return this[HANDLE];
    }

    /** true после close(). / true after close(). */
    get closed()
    {
        return this[HANDLE] === null;
    }

    /** Освобождение нативной памяти; повторный вызов безопасен. / Releases the native memory; calling it again is safe. */
    close()
    {
        if (this[HANDLE] === null)
            return;
        const handle = this[HANDLE];
        this[HANDLE] = null;
        release(this, 'xpath_free', handle);
    }

    /** Поддержка `using`. / Support for `using`. */
    [Symbol.dispose]()
    {
        this.close();
    }
}

applyMixins(XPath, XPathContext, XPathEvaluation, XPathScalars);

module.exports = { XPath };
