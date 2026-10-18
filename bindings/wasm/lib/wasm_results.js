'use strict';

const { BerestaError } = require('../../node/lib/errors.js');

/*
 * Последняя ошибка библиотеки -> BerestaError(status, code, message) — так
 * же, как это делает аддон Node.js (errors.cpp): JS-слой и пользовательский
 * код не видят разницы между бэкендами.
 *
 * The last library error -> BerestaError(status, code, message) — the same
 * way the Node.js addon does it (errors.cpp): the JS layer and user
 * code see no difference between the backends.
 */
function throwLastError(core)
{
    const status = core._beresta_last_error_code();
    const code = core.UTF8ToString(core._beresta_status_name(status));
    const message = core.UTF8ToString(core._beresta_last_error_message());

    throw new BerestaError(status, code, message);
}

// Указатель -> дескриптор BigInt; NULL — null или ошибка, если она записана
// A pointer -> a BigInt handle; NULL is null or an error if one was recorded
function toHandle(core, pointer)
{
    if (pointer === 0)
    {
        if (core._beresta_last_error_code() !== 0)
            throwLastError(core);
        return null;
    }
    return BigInt(pointer >>> 0);
}

// Строка библиотеки -> строка JS; owned освобождается сразу после копирования
// A library string -> a JS string; owned is freed right after copying
function toText(core, pointer, owned)
{
    if (pointer === 0)
    {
        if (core._beresta_last_error_code() !== 0)
            throwLastError(core);
        return null;
    }
    const text = core.UTF8ToString(pointer);
    if (owned)
        core._beresta_string_free(pointer);
    return text;
}

// Числовые виды результата: отрицательное значение значит ошибку
// Numeric result kinds: a negative value means an error
function makeNumericResult(core, kind, raw, scope)
{
    if (kind === 'outNumber')
    {
        if (raw !== 0)
            throwLastError(core);
        return core.getValue(scope.slot, 'double');
    }
    const value = Number(raw);
    if (value < 0)
        throwLastError(core);
    if (kind === 'flag')
        return value !== 0;
    return value;
}

// Результат вызова по виду из сигнатуры / The call result by the kind from the signature
function makeResult(core, kind, raw, scope)
{
    if (kind === 'void')
        return undefined;
    if (kind === 'borrowed' || kind === 'owned')
        return toText(core, raw, kind === 'owned');
    if (kind === 'handle')
        return toHandle(core, raw);
    if (kind === 'status' || kind === 'out')
    {
        if (raw !== 0)
            throwLastError(core);
        if (kind === 'status')
            return undefined;
        return toHandle(core, core.getValue(scope.slot, 'i32'));
    }
    return makeNumericResult(core, kind, raw, scope);
}

module.exports = { makeResult, throwLastError };
