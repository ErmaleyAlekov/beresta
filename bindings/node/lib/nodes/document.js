'use strict';

const { applyMixins } = require('../internal/mixin.js');
const { HANDLE, OWNER, REGISTRY } = require('../internal/keys.js');
const { track, release } = require('../internal/release.js');
const { XmlNode } = require('./node.js');
const { DocumentContent } = require('./document_content.js');
const { NodeRegistry } = require('./registry.js');

/**
 * XML-документ — владелец всего дерева узлов в нативной памяти. Документ
 * сам является узлом (к нему можно добавлять детей). Память освобождается
 * детерминированно — close() или `using` — либо, если про это забыли,
 * при сборке мусора (FinalizationRegistry, см. release.js).
 *
 * An XML document — the owner of the whole node tree in native memory. The
 * document is itself a node (children can be appended to it). The memory is
 * freed deterministically — close() or `using` — or, if that was forgotten,
 * on garbage collection (FinalizationRegistry, see release.js).
 */
class XmlDocument extends XmlNode
{
    /** Создаётся фабриками parse / parseFile / createDocument. / Created by the parse / parseFile / createDocument factories. */
    constructor(handle)
    {
        super(null, handle);
        this[OWNER] = this;
        this[REGISTRY] = new NodeRegistry();
        track(this, 'document_free', handle);
    }

    /** true после close(): память документа уже освобождена. / true after close(): the document memory is already freed. */
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
        release(this, 'document_free', handle);
    }

    /** Поддержка `using document = beresta.parse(...)`. / Support for `using document = beresta.parse(...)`. */
    [Symbol.dispose]()
    {
        this.close();
    }
}

applyMixins(XmlDocument, DocumentContent);

module.exports = { XmlDocument };
