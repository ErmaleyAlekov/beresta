'use strict';

/**
 * Ошибка «Берёсты». status — число beresta_status Си-интерфейса,
 * code — имя кода (PARSE_ERROR, VALIDATION_ERROR...), одинаковое во всех
 * языках, — по соглашению Node.js, как ENOENT у модуля fs. message — текст
 * на текущем языке сообщений.
 *
 * A Beresta error. status is the beresta_status number of the C interface,
 * code is the code name (PARSE_ERROR, VALIDATION_ERROR...), the same in all
 * languages — by the Node.js convention, like ENOENT of the fs module. message
 * is the text in the current message language.
 */
class BerestaError extends Error
{
    constructor(status, code, message)
    {
        super(message);
        this.name = 'BerestaError';
        this.status = status;
        this.code = code;
    }
}

/**
 * Обращение к уже закрытому документу, выражению или схеме. Это ошибка
 * программы, а не данных, поэтому у неё отдельный класс.
 *
 * An access to an already closed document, expression or schema. This is a
 * program error, not a data error, so it has a separate class.
 */
class BerestaClosedError extends Error
{
    constructor(message)
    {
        super(message);
        this.name = 'BerestaClosedError';
        this.code = 'OBJECT_CLOSED';
    }
}

/**
 * Нативный аддон не найден или не загрузился. Текст двуязычный: каталог
 * сообщений живёт в самом аддоне и до его загрузки недоступен.
 *
 * The native addon is not found or failed to load. The text is bilingual:
 * the message catalog lives in the addon itself and is unavailable before it loads.
 */
class BerestaLoadError extends Error
{
    constructor(message)
    {
        super(message);
        this.name = 'BerestaLoadError';
        this.code = 'ADDON_NOT_LOADED';
    }
}

module.exports = { BerestaError, BerestaClosedError, BerestaLoadError };
