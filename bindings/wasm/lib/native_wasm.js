'use strict';

const { SIGNATURES, BINDINGS } = require('./wasm_signatures.js');
const { invokeBinding } = require('./wasm_call.js');
const { BerestaLoadError } = require('../../node/lib/errors.js');

/*
 * Бэкенд WebAssembly для общего JS-слоя «Берёсты» (паттерн Adapter). Для
 * классов узлов, XPath и схем он выглядит так же, как аддон Node.js:
 * объект native с теми же функциями, дескрипторы — BigInt, ошибки —
 * BerestaError. Сборщик (tools/bundle.mjs) подставляет этот модуль вместо
 * internal/native.js, поэтому классы не дублируются.
 *
 * Модуль Wasm создаётся асинхронно, а функции нужны модулям при загрузке
 * как свойства объекта, — поэтому объект native создаётся пустым сразу, а
 * install заполняет его, когда модуль Wasm готов.
 *
 * The WebAssembly backend for the common Beresta JS layer (the Adapter pattern). For
 * the node, XPath and schema classes it looks the same as the Node.js addon:
 * a native object with the same functions, handles are BigInt, errors are
 * BerestaError. The bundler (tools/bundle.mjs) substitutes this module for
 * internal/native.js, so the classes are not duplicated.
 *
 * The Wasm module is created asynchronously, while the modules need the functions at load time
 * as properties of an object — that is why the native object is created empty at once, and
 * install fills it when the Wasm module is ready.
 */
const native = {};

// Имя платформы для сообщений и platformName() / The platform name for messages and platformName()
function platformName()
{
    return 'wasm32';
}

/*
 * Заполнение native функциями модуля Wasm core. Функция, которой нет в
 * модуле (exports.json разошёлся с таблицей BINDINGS), — ошибка загрузки
 * сразу, а не TypeError при первом вызове где-то в глубине программы.
 *
 * Fills native with the functions of the core Wasm module. A function missing from the
 * module (exports.json went out of sync with the BINDINGS table) is a loading error
 * at once, not a TypeError on the first call somewhere deep in the program.
 */
function install(core)
{
    for (const [name, signature] of BINDINGS)
    {
        const target = core['_beresta_' + name];
        if (typeof target !== 'function')
            throw new BerestaLoadError('В модуле WebAssembly нет функции / The WebAssembly module lacks the function '
                + 'beresta_' + name + ' (bindings/wasm/exports.json)');
        native[name] = invokeBinding.bind(null, { name, signature: SIGNATURES[signature], target, core });
    }
}

module.exports = { native, platformName, install };
