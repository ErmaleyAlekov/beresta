'use strict';

/*
 * Поиск и загрузка нативного аддона «Берёсты» (beresta.node).
 * Порядок поиска (первый найденный файл побеждает):
 *   1. переменная окружения BERESTA_NODE_ADDON — явный путь к файлу;
 *   2. папка пакета native/<os>-<arch>/ — туда бинарник кладут
 *      CMake (-DBERESTA_BUILD_NODE=ON) и Makefile (make NODE=1).
 * Имена ОС и архитектуры совпадают с CMake, Makefile и остальными обёртками:
 * windows / linux / macos и x86_64 / aarch64 / x86.
 * Модуль загружается один раз (кэш require — паттерн Singleton).
 *
 * Finding and loading the Beresta native addon (beresta.node).
 * Lookup order (the first file found wins):
 *   1. the BERESTA_NODE_ADDON environment variable — an explicit file path;
 *   2. the package folder native/<os>-<arch>/ — the binary is put there by
 *      CMake (-DBERESTA_BUILD_NODE=ON) and the Makefile (make NODE=1).
 * The OS and architecture names match CMake, the Makefile and the other bindings:
 * windows / linux / macos and x86_64 / aarch64 / x86.
 * The module is loaded once (the require cache — the Singleton pattern).
 */

const fs = require('node:fs');
const path = require('node:path');
const { BerestaError, BerestaLoadError } = require('../errors.js');

// Имена ОС Node.js -> имена папок / Node.js OS names -> folder names
const OS_NAMES = Object.freeze({ win32: 'windows', linux: 'linux', darwin: 'macos' });

// Имена архитектур Node.js -> имена папок / Node.js architecture names -> folder names
const ARCH_NAMES = Object.freeze({ x64: 'x86_64', arm64: 'aarch64', ia32: 'x86' });

// Имя платформы вида windows-x86_64 (имя папки в native/)
// The platform name like windows-x86_64 (the folder name in native/)
function platformName()
{
    const os = OS_NAMES[process.platform] || process.platform;
    const arch = ARCH_NAMES[process.arch] || process.arch;

    return os + '-' + arch;
}

// Кандидаты в порядке приоритета / Candidates in priority order
function candidates()
{
    const result = [];

    if (process.env.BERESTA_NODE_ADDON)
        result.push(path.resolve(process.env.BERESTA_NODE_ADDON));
    result.push(path.join(__dirname, '..', '..', 'native', platformName(), 'beresta.node'));
    return result;
}

/*
 * Загрузка аддона и передача ему класса ошибки: дальше аддон бросает
 * BerestaError сам. Аддон без функции init значит, что файл — не
 * аддон «Берёсты» или Node-API в процессе недоступен.
 *
 * Loads the addon and passes the error class to it: from then on the addon
 * throws BerestaError itself. An addon without the init function means that
 * the file is not a Beresta addon or Node-API is unavailable in the process.
 */
function loadNative()
{
    const tried = candidates();

    for (const file of tried)
    {
        if (!fs.existsSync(file))
            continue;
        const native = require(file);
        if (typeof native.init !== 'function')
            throw new BerestaLoadError('Аддон не зарегистрировал функции Node-API / '
                + 'The addon did not register its Node-API functions: ' + file);
        native.init(BerestaError);
        return native;
    }
    throw new BerestaLoadError('Нативный аддон «Берёсты» для ' + platformName() + ' не найден / '
        + 'The Beresta native addon for ' + platformName() + ' was not found: ' + tried.join(', ')
        + '. Соберите его / Build it: make NODE=1 или / or cmake -DBERESTA_BUILD_NODE=ON');
}

module.exports = { native: loadNative(), platformName };
