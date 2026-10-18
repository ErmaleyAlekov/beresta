'use strict';

const { HANDLE, OWNER } = require('../internal/keys.js');
const { BerestaClosedError } = require('../errors.js');
const { messageText, Messages } = require('../languages.js');

/**
 * Основа узла: нативный дескриптор и документ-владелец. Ссылка на
 * документ решает две задачи: пока жив узел, документ не соберёт сборщик
 * мусора, а после document.close() обращение к узлу даёт понятную ошибку
 * вместо падения процесса.
 *
 * The base of a node: the native handle and the owner document. The reference
 * to the document solves two problems: while the node is alive, the document is
 * not garbage-collected, and after document.close() any access to the node gives
 * a clear error instead of a process crash.
 */
class NodeBase
{
    constructor(document, handle)
    {
        this[OWNER] = document;
        this[HANDLE] = handle;
    }

    /** Дескриптор для вызова аддона; бросает BerestaClosedError, если документ закрыт.
     *  The handle for addon calls; throws BerestaClosedError if the document is closed. */
    get handle()
    {
        if (this[OWNER].closed)
            throw new BerestaClosedError(messageText(Messages.DOCUMENT_CLOSED));
        return this[HANDLE];
    }

    /** Документ, которому принадлежит узел. / The document the node belongs to. */
    get document()
    {
        return this[OWNER];
    }

    /** Указывают ли две обёртки на один и тот же узел (как isSameNode в DOM).
     *  Whether two wrappers point to the same node (like isSameNode in DOM). */
    isSameNode(other)
    {
        return other instanceof NodeBase && other[HANDLE] === this[HANDLE] && other[HANDLE] !== null;
    }

    /** Короткое представление для console.log и util.inspect: «XmlElement <item>».
     *  A short representation for console.log and util.inspect: "XmlElement <item>". */
    [Symbol.for('nodejs.util.inspect.custom')]()
    {
        if (this[OWNER].closed)
            return this.constructor.name + ' (closed)';
        return this.constructor.name + ' <' + this.name + '>';
    }
}

module.exports = { NodeBase };
