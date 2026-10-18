'use strict';

const { native } = require('./internal/native.js');
const { toBytes } = require('./internal/bytes.js');
const { ParseFlags } = require('./flags.js');
const { XmlDocument } = require('./nodes/document.js');

// Дескриптор схемы из настроек или null (без проверки по схеме)
// The schema handle from the options or null (no schema validation)
function schemaHandle(options)
{
    if (options.schema === undefined || options.schema === null)
        return null;
    return options.schema.handle;
}

/**
 * Разбор XML. Строка кодируется в UTF-8; Buffer / Uint8Array передаются как
 * есть — кодировку (UTF-16, windows-1251...) библиотека определит сама.
 * options.flags — ParseFlags, options.schema — проверка по XML Schema.
 *
 * Parses XML. A string is encoded to UTF-8; a Buffer / Uint8Array is passed as
 * is — the library detects the encoding (UTF-16, windows-1251...) itself.
 * options.flags are ParseFlags, options.schema is validation against an XML Schema.
 */
function parse(input, options = {})
{
    const bytes = toBytes(input, 'parse');
    const flags = options.flags ?? ParseFlags.DEFAULT;

    return new XmlDocument(native.document_parse(bytes, flags, schemaHandle(options)));
}

/** Разбор файла; ParseFlags.FILE_RESOURCES разрешает внешние DTD рядом с ним.
 *  Parses a file; ParseFlags.FILE_RESOURCES allows external DTDs next to it. */
function parseFile(path, options = {})
{
    const flags = options.flags ?? ParseFlags.DEFAULT;

    return new XmlDocument(native.document_parse_file(String(path), flags, schemaHandle(options)));
}

/** Новый документ; rootName === null — без корневого элемента.
 *  A new document; rootName === null means without a root element. */
function createDocument(rootName = null)
{
    return new XmlDocument(native.document_create(rootName));
}

module.exports = { parse, parseFile, createDocument };
