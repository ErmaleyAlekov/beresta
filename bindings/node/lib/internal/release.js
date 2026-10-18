'use strict';

/*
 * Освобождение нативных объектов (документ, XPath-выражение, схема).
 * Основной путь — явный close() или `using` (Symbol.dispose). Страховка —
 * FinalizationRegistry: если объект забыли закрыть, нативная память всё
 * равно освободится, когда сборщик мусора удалит обёртку. В реестре хранится
 * только имя функции и дескриптор, а не сама обёртка: иначе она никогда
 * не стала бы недостижимой. После close() запись снимается — двойного
 * освобождения не бывает.
 *
 * Releasing native objects (a document, an XPath expression, a schema).
 * The main path is an explicit close() or `using` (Symbol.dispose). The safety
 * net is FinalizationRegistry: if an object was not closed, the native memory
 * is still freed when the garbage collector removes the wrapper. The registry
 * keeps only the function name and the handle, not the wrapper itself:
 * otherwise it would never become unreachable. After close() the entry is
 * removed — a double free never happens.
 */

const { native } = require('./native.js');

// Освобождение по записи реестра / Releasing by a registry entry
function releaseHandle(held)
{
    native[held.free](held.handle);
}

const registry = new FinalizationRegistry(releaseHandle);

// Владелец будет освобождён функцией free при сборке мусора / The owner will be freed by free on garbage collection
function track(owner, free, handle)
{
    registry.register(owner, { free, handle }, owner);
}

// Явное освобождение: запись снимается, затем вызывается free / An explicit release: the entry is removed, then free is called
function release(owner, free, handle)
{
    registry.unregister(owner);
    native[free](handle);
}

module.exports = { track, release };
