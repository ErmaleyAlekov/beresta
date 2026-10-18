'use strict';

const { messageText, Messages } = require('../languages.js');

/*
 * Вход разбора -> байты для аддона. Строка кодируется в UTF-8; Buffer и
 * Uint8Array передаются как есть, без копирования, — кодировку (UTF-16,
 * windows-1251...) ядро определит само по BOM и XML-декларации.
 * ArrayBuffer оборачивается представлением Uint8Array (тоже без копии).
 *
 * The parse input -> bytes for the addon. A string is encoded to UTF-8; a Buffer
 * and a Uint8Array are passed as is, without copying — the core detects the
 * encoding (UTF-16, windows-1251...) itself by the BOM and the XML declaration.
 * An ArrayBuffer is wrapped in a Uint8Array view (also without a copy).
 */
function toBytes(input, functionName)
{
    if (typeof input === 'string')
        return Buffer.from(input, 'utf8');
    if (input instanceof Uint8Array)
        return input;
    if (input instanceof ArrayBuffer)
        return new Uint8Array(input);
    throw new TypeError(messageText(Messages.INVALID_ARGUMENT, 1, functionName,
        'string | Buffer | Uint8Array | ArrayBuffer'));
}

module.exports = { toBytes };
