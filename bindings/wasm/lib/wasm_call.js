'use strict';

const { CallScope } = require('./wasm_scope.js');
const { makeResult } = require('./wasm_results.js');

/*
 * Неверный аргумент -> TypeError с кодом INVALID_ARGUMENT и текстом из
 * каталога сообщений — как у аддона Node.js. languages.js подключается
 * при первой ошибке, а не при загрузке: он сам зависит от этого бэкенда.
 *
 * An invalid argument -> a TypeError with the INVALID_ARGUMENT code and the text from
 * the message catalog — as in the Node.js addon. languages.js is loaded
 * on the first error, not at load time: it depends on this backend itself.
 */
function rejectArgument(binding, position, expected)
{
    const { messageText, Messages } = require('../../node/lib/languages.js');
    const error = new TypeError(messageText(Messages.INVALID_ARGUMENT, position + 1, binding.name, expected));

    error.code = 'INVALID_ARGUMENT';
    throw error;
}

// Числовые виды и дескрипторы -> значения для Wasm (int64 — BigInt)
// Numeric kinds and handles -> values for Wasm (int64 is a BigInt)
function convertScalar(binding, kind, value, position)
{
    if (kind === 'handle')
    {
        if (value === null || value === undefined)
            return 0;
        if (typeof value !== 'bigint')
            rejectArgument(binding, position, 'BigInt');
        return Number(value);
    }
    if (kind === 'flags' && value === undefined)
        return 0;
    if (typeof value !== 'number')
        rejectArgument(binding, position, 'number');
    if (kind === 'index')
    {
        if (!Number.isFinite(value))
            return 0n;
        return BigInt(Math.trunc(value));
    }
    if (kind === 'flags')
        return value >>> 0;
    if (kind === 'int')
        return value | 0;
    return value;
}

// Строки, байты и служебные ячейки -> указатели в памяти Wasm
// Strings, bytes and service slots -> pointers in the Wasm memory
function convertMemory(binding, scope, kind, value, position)
{
    if (kind === 'out' || kind === 'outNumber')
    {
        scope.slot = scope.allocate(8);
        scope.core.HEAPU8.fill(0, scope.slot, scope.slot + 8);
        return [scope.slot];
    }
    if (kind === 'bytes')
    {
        if (!(value instanceof Uint8Array))
            rejectArgument(binding, position, 'Uint8Array');
        return [scope.putBytes(value), BigInt(value.length)];
    }
    if (value === null || value === undefined)
        return [0];
    if (typeof value !== 'string')
        rejectArgument(binding, position, 'string');
    return [scope.putString(value)];
}

// Аргументы JS -> список аргументов функции Си (байты дают два: указатель и длину)
// JS arguments -> the list of the C function arguments (bytes give two: a pointer and a length)
function convertArguments(binding, scope, values)
{
    const result = [];
    let position = 0;

    for (const kind of binding.signature.args)
    {
        if (kind === 'string' || kind === 'bytes' || kind === 'out' || kind === 'outNumber')
            result.push(...convertMemory(binding, scope, kind, values[position], position));
        else
            result.push(convertScalar(binding, kind, values[position], position));
        if (kind !== 'out' && kind !== 'outNumber')
            ++position;
    }
    return result;
}

/*
 * Общий вызов любой функции Си-интерфейса по её сигнатуре. Привязывается к
 * описанию функции через bind (см. native_wasm.js), поэтому для 66 функций
 * не нужно 66 обёрток.
 *
 * The common call of any C interface function by its signature. It is bound to
 * the function description through bind (see native_wasm.js), so 66 functions
 * do not need 66 wrappers.
 */
function invokeBinding(binding, ...values)
{
    const scope = new CallScope(binding.core);

    try
    {
        const raw = binding.target(...convertArguments(binding, scope, values));
        return makeResult(binding.core, binding.signature.result, raw, scope);
    }
    finally
    {
        scope.release();
    }
}

module.exports = { invokeBinding };
