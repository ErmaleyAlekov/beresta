'use strict';

const { native } = require('../internal/native.js');
const { HANDLE } = require('../internal/keys.js');
const { track } = require('../internal/release.js');
const { toBytes } = require('../internal/bytes.js');
const { applyMixins } = require('../internal/mixin.js');
const { BerestaClosedError } = require('../errors.js');
const { messageText, Messages } = require('../languages.js');
const { readDiagnostics } = require('./diagnostics.js');
const { SchemaLifecycle } = require('./schema_lifecycle.js');

/**
 * Загруженная XML Schema. Неизменяема после загрузки: одну схему можно
 * использовать для множества документов — и при разборе (parse с
 * options.schema), и отдельной проверкой validate.
 *
 * A loaded XML Schema. Immutable after loading: one schema can be used for
 * many documents — both during parsing (parse with options.schema) and
 * as a separate validate check.
 */
class Schema
{
    /** Создаётся фабриками fromString / fromFile. / Created by the fromString / fromFile factories. */
    constructor(handle)
    {
        this[HANDLE] = handle;
        track(this, 'schema_free', handle);
    }

    /** Схема из текста или байтов XSD (xs:include здесь недоступен — нет каталога).
     *  A schema from XSD text or bytes (xs:include is unavailable here — there is no directory). */
    static fromString(input)
    {
        return new Schema(native.schema_load(toBytes(input, 'Schema.fromString')));
    }

    /** Схема из файла; xs:include / xs:import ищутся рядом с ним.
     *  A schema from a file; xs:include / xs:import are looked up next to it. */
    static fromFile(path)
    {
        return new Schema(native.schema_load_file(String(path)));
    }

    /** Дескриптор; BerestaClosedError, если схема уже закрыта. / The handle; BerestaClosedError if the schema is closed. */
    get handle()
    {
        if (this[HANDLE] === null)
            throw new BerestaClosedError(messageText(Messages.SCHEMA_CLOSED));
        return this[HANDLE];
    }

    /** Все нарушения за один проход; пустой массив — документ корректен.
     *  All violations in one pass; an empty array means the document is valid. */
    validate(document)
    {
        return readDiagnostics(native.schema_validate(this.handle, document.handle));
    }
}

applyMixins(Schema, SchemaLifecycle);

module.exports = { Schema };
