'use strict';

/*
 * Языки сообщений. Язык общий для всего процесса: его смена сразу меняет
 * тексты ошибок ядра и сообщения самой обёртки. Коды ошибок (PARSE_ERROR...)
 * от языка не зависят.
 *
 * Message languages. The language is shared by the whole process: changing it
 * changes the core error texts and the messages of the binding itself at once.
 * Error codes (PARSE_ERROR...) do not depend on the language.
 */

const { native } = require('./internal/native.js');

// Номера сообщений обёрток (beresta_message) / Binding message numbers (beresta_message)
const Messages = Object.freeze({
    DOCUMENT_CLOSED: 1,
    XPATH_CLOSED: 2,
    SCHEMA_CLOSED: 3,
    NATIVE_CALL_FAILED: 4,
    UNKNOWN_NODE_TYPE: 5,
    INVALID_ARGUMENT: 6,
    NO_FILE_SYSTEM: 7
});

// Смена языка; неизвестный код — BerestaError(INVALID_ARGUMENT) / Switches the language; an unknown code is BerestaError(INVALID_ARGUMENT)
function setLanguage(code)
{
    native.set_language(code);
}

// Код текущего языка, например "ru" / The current language code, for example "ru"
function language()
{
    return native.language();
}

// Все языки библиотеки: ["ru", "en", ...] / All languages of the library: ["ru", "en", ...]
function languages()
{
    return native.languages().split(',');
}

/*
 * Сообщение обёртки на текущем языке; метки {0}, {1}... заменяются
 * аргументами по номеру (метка без аргумента остаётся видна как есть).
 *
 * A binding message in the current language; the {0}, {1}... placeholders are
 * replaced with the arguments by number (a placeholder without an argument stays visible as is).
 */
function messageText(message, ...values)
{
    const template = native.message_text(message);
    let result = '';
    let position = 0;

    for (const match of template.matchAll(/\{(\d+)\}/g))
    {
        const index = Number(match[1]);
        result += template.slice(position, match.index);
        if (index < values.length)
            result += String(values[index]);
        else
            result += match[0];
        position = match.index + match[0].length;
    }
    return result + template.slice(position);
}

module.exports = { Messages, setLanguage, language, languages, messageText };
