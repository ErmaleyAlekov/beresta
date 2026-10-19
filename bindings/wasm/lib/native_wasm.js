'use strict';

const { SIGNATURES, BINDINGS } = require('./wasm_signatures.js');
const { invokeBinding } = require('./wasm_call.js');
const { BerestaError, BerestaLoadError } = require('../../node/lib/errors.js');

// Код BERESTA_IO_ERROR / The BERESTA_IO_ERROR code
const IO_ERROR = 9;

// Функции, которым нужна файловая система / The functions that need a file system
const FILE_FUNCTIONS = Object.freeze(['document_parse_file', 'document_save', 'schema_load_file', 'schema_validate_file']);

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
 * Модуль собран без файловой системы (-sFILESYSTEM=0), и тогда Emscripten
 * «открывает» любой путь и отдаёт пустое содержимое: отсутствующий файл
 * выглядел бы как PARSE_ERROR «нет корня». Поэтому файловые функции
 * отказывают сразу и честно — IO_ERROR с объяснением на текущем языке.
 * languages.js загружается при вызове, а не при загрузке модуля: он сам
 * зависит от native, и раннее подключение дало бы цикл.
 *
 * The module is built without a file system (-sFILESYSTEM=0), and then Emscripten
 * "opens" any path and returns empty content: a missing file would look like
 * a PARSE_ERROR "no root". That is why the file functions refuse at once and
 * honestly — IO_ERROR with an explanation in the current language.
 * languages.js is loaded on the call, not at module load: it depends on native
 * itself, and an early require would make a cycle.
 */
function refuseFileSystem(name)
{
    const { messageText, Messages } = require('../../node/lib/languages.js');

    throw new BerestaError(IO_ERROR, 'IO_ERROR', messageText(Messages.NO_FILE_SYSTEM, 'beresta_' + name));
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
    // Файловые функции без файловой системы отказывают сразу (см. refuseFileSystem)
    // The file functions without a file system refuse at once (see refuseFileSystem)
    for (const name of FILE_FUNCTIONS)
        native[name] = refuseFileSystem.bind(null, name);
}

module.exports = { native, platformName, install };
