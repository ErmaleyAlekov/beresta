'use strict';

const { HANDLE } = require('../internal/keys.js');
const { release } = require('../internal/release.js');

/**
 * Примесь схемы: закрытие и поддержка `using`.
 * A schema mixin: closing and `using` support.
 */
class SchemaLifecycle
{
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
        release(this, 'schema_free', handle);
    }

    /** Поддержка `using`. / Support for `using`. */
    [Symbol.dispose]()
    {
        this.close();
    }
}

module.exports = { SchemaLifecycle };
