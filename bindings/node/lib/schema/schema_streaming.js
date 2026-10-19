'use strict';

const { native } = require('../internal/native.js');
const { toBytes } = require('../internal/bytes.js');
const { ParseFlags } = require('../flags.js');
const { readDiagnostics } = require('./diagnostics.js');

/**
 * Примесь схемы: потоковая проверка без построения дерева (SAX + XSD внутри
 * ядра). Память — O(глубины документа) плюс значения ID и таблицы ключей;
 * файл отображается в память, а не копируется. Так проверяются выгрузки в
 * гигабайты, и это быстрее, чем parse + validate: дерево не строится вовсе.
 * Нарушения схемы возвращаются массивом, синтаксическая ошибка XML —
 * исключение BerestaError (PARSE_ERROR).
 *
 * A schema mixin: streaming validation without building a tree (SAX + XSD inside
 * the core). The memory is O(document depth) plus ID values and key tables;
 * the file is mapped into memory, not copied. This is how exports of gigabytes
 * are validated, and it is faster than parse + validate: no tree is built at all.
 * Schema violations are returned as an array, an XML syntax error is a
 * BerestaError (PARSE_ERROR).
 */
class SchemaStreaming
{
    /** Проверка XML из памяти (строка, Buffer или Uint8Array); options.flags — ParseFlags.
     *  Validates XML from memory (a string, a Buffer or a Uint8Array); options.flags are ParseFlags. */
    validateBytes(input, options = {})
    {
        const bytes = toBytes(input, 'Schema.validateBytes');
        const flags = options.flags ?? ParseFlags.DEFAULT;

        return readDiagnostics(native.schema_validate_bytes(bytes, flags, this.handle));
    }

    /** Проверка файла; ParseFlags.FILE_RESOURCES разрешает внешние DTD рядом с ним.
     *  Validates a file; ParseFlags.FILE_RESOURCES allows external DTDs next to it. */
    validateFile(path, options = {})
    {
        const flags = options.flags ?? ParseFlags.DEFAULT;

        return readDiagnostics(native.schema_validate_file(String(path), flags, this.handle));
    }
}

module.exports = { SchemaStreaming };
