'use strict';

const { native } = require('../internal/native.js');
const { WriteFlags } = require('../flags.js');
const { wrap } = require('./wrapping.js');

/**
 * Примесь документа: корень, поиск по ID, копия, запись в файл.
 * A document mixin: the root, lookup by ID, a copy, writing to a file.
 */
class DocumentContent
{
    /** Корневой элемент или null. / The root element or null. */
    get root()
    {
        return wrap(this, native.document_root(this.handle));
    }

    /** Элемент с атрибутом типа ID (по DTD) или xml:id; null, если не найден.
     *  An element with an ID-typed attribute (by DTD) or xml:id; null if not found. */
    elementById(id)
    {
        return wrap(this, native.document_element_by_id(this.handle, id));
    }

    /** Независимая глубокая копия; её тоже нужно закрыть. / An independent deep copy; it must be closed too. */
    clone()
    {
        return new this.constructor(native.document_clone(this.handle));
    }

    /**
     * Запись в файл. options.flags — WriteFlags, options.encoding — имя
     * кодировки (по умолчанию UTF-8), например "windows-1251".
     *
     * Writes to a file. options.flags are WriteFlags, options.encoding is an
     * encoding name (UTF-8 by default), for example "windows-1251".
     */
    save(path, options = {})
    {
        const flags = options.flags ?? WriteFlags.DEFAULT;
        const encoding = options.encoding ?? null;

        native.document_save(this.handle, String(path), flags, encoding);
    }
}

module.exports = { DocumentContent };
