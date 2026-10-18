'use strict';

const { native } = require('../internal/native.js');

/**
 * Одно нарушение схемы: текст начинается с пути к элементу
 * ("/catalog/book[2]: ..."), line и column — место в исходнике (0 — неизвестно).
 *
 * One schema violation: the text starts with the element path
 * ("/catalog/book[2]: ..."), line and column are the place in the source (0 — unknown).
 */
class Diagnostic
{
    constructor(message, line, column)
    {
        this.message = message;
        this.line = line;
        this.column = column;
        Object.freeze(this);
    }
}

/*
 * Чтение нативного списка нарушений в массив; список освобождается в
 * finally — даже если чтение прервало исключение.
 *
 * Reads the native list of violations into an array; the list is freed in
 * finally — even if reading was interrupted by an exception.
 */
function readDiagnostics(list)
{
    const result = [];

    try
    {
        const count = native.diagnostics_count(list);
        for (let index = 0; index < count; ++index)
        {
            result.push(new Diagnostic(native.diagnostics_message(list, index),
                native.diagnostics_line(list, index), native.diagnostics_column(list, index)));
        }
    }
    finally
    {
        native.diagnostics_free(list);
    }
    return result;
}

module.exports = { Diagnostic, readDiagnostics };
