'use strict';

/*
 * Ключи внутренних полей объектов-обёрток. Символы вместо #приватных полей:
 * методы примесей (mixin.js) определены в других классах, а доступ к
 * #полю разрешён только внутри класса, который его объявил. Символ не
 * виден в Object.keys и JSON, поэтому поля всё равно скрыты от пользователя.
 *
 * The keys of the internal fields of the wrapper objects. Symbols instead of
 * #private fields: the mixin methods (mixin.js) are defined in other classes,
 * and a #field is accessible only inside the class that declared it. A symbol
 * is invisible in Object.keys and JSON, so the fields are still hidden from the user.
 */

// Нативный дескриптор (BigInt; null после закрытия) / The native handle (BigInt; null after closing)
const HANDLE = Symbol('beresta.handle');

// Документ-владелец узла / The owner document of a node
const OWNER = Symbol('beresta.owner');

// Реестр обёрток узлов документа (см. nodes/registry.js) / The node wrapper registry of a document (see nodes/registry.js)
const REGISTRY = Symbol('beresta.registry');

module.exports = { HANDLE, OWNER, REGISTRY };
