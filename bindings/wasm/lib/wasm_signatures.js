'use strict';

/*
 * Сигнатуры функций Си-интерфейса для вызова из JavaScript через WebAssembly.
 * Каждая сигнатура — данные, а не код (паттерн Interpreter): список видов
 * аргументов и вид результата. Общая функция вызова (wasm_call.js) по этим
 * описаниям сама кладёт аргументы в память Wasm и разбирает результат —
 * поэтому 31 сигнатура аддона Node.js здесь не превращается в 31 функцию.
 *
 * Виды аргументов:
 *   handle — дескриптор (BigInt, null или undefined -> NULL);
 *   string — строка UTF-8 (null или undefined -> NULL);
 *   index  — int64 (number);  int — int32 (number);  number — double;
 *   flags  — uint32 (undefined -> 0);
 *   bytes  — Uint8Array -> указатель и длина int64;
 *   out    — служебная ячейка под выходной указатель (из JS не передаётся);
 *   outNumber — служебная ячейка под double.
 * Виды результата:
 *   borrowed / owned — строка библиотеки (owned освобождается после чтения);
 *   status — код возврата (не 0 -> исключение);  out / outNumber — код
 *   возврата и значение из ячейки;  handle — дескриптор или null;
 *   count / integer — неотрицательное число;  flag — логическое;  void.
 *
 * The signatures of the C interface functions for calls from JavaScript through WebAssembly.
 * Each signature is data, not code (the Interpreter pattern): a list of argument
 * kinds and a result kind. The common call function (wasm_call.js) uses these
 * descriptions to put the arguments into the Wasm memory and to parse the result —
 * that is why the 31 signatures of the Node.js addon do not turn into 31 functions here.
 *
 * Argument kinds:
 *   handle — a handle (BigInt, null or undefined -> NULL);
 *   string — a UTF-8 string (null or undefined -> NULL);
 *   index  — int64 (number);  int — int32 (number);  number — double;
 *   flags  — uint32 (undefined -> 0);
 *   bytes  — Uint8Array -> a pointer and an int64 length;
 *   out    — a service slot for an output pointer (not passed from JS);
 *   outNumber — a service slot for a double.
 * Result kinds:
 *   borrowed / owned — a library string (owned is freed after reading);
 *   status — a return code (not 0 -> an exception);  out / outNumber — the return
 *   code and the value from the slot;  handle — a handle or null;
 *   count / integer — a non-negative number;  flag — a boolean;  void.
 */
const SIGNATURES = Object.freeze({
    borrowed: { args: [], result: 'borrowed' },
    borrowedOfInt: { args: ['int'], result: 'borrowed' },
    statusOfString: { args: ['string'], result: 'status' },
    outOfString: { args: ['string', 'out'], result: 'out' },
    voidOfHandle: { args: ['handle'], result: 'void' },
    outOfHandle: { args: ['handle', 'out'], result: 'out' },
    handleOfHandle: { args: ['handle'], result: 'handle' },
    handleOfHandleString: { args: ['handle', 'string'], result: 'handle' },
    handleOfHandleIndex: { args: ['handle', 'index'], result: 'handle' },
    handleOfHandleStrings: { args: ['handle', 'string', 'string'], result: 'handle' },
    statusOfHandle: { args: ['handle'], result: 'status' },
    integerOfHandle: { args: ['handle'], result: 'integer' },
    statusOfHandleString: { args: ['handle', 'string'], result: 'status' },
    statusOfHandleStrings: { args: ['handle', 'string', 'string'], result: 'status' },
    statusOfHandleStringNumber: { args: ['handle', 'string', 'number'], result: 'status' },
    statusOfHandles: { args: ['handle', 'handle'], result: 'status' },
    ownedOfHandle: { args: ['handle'], result: 'owned' },
    ownedOfHandleFlags: { args: ['handle', 'flags'], result: 'owned' },
    ownedOfHandleString: { args: ['handle', 'string'], result: 'owned' },
    ownedOfHandleIndex: { args: ['handle', 'index'], result: 'owned' },
    ownedOfHandles: { args: ['handle', 'handle'], result: 'owned' },
    countOfHandle: { args: ['handle'], result: 'count' },
    countOfHandleIndex: { args: ['handle', 'index'], result: 'count' },
    flagOfHandleString: { args: ['handle', 'string'], result: 'flag' },
    flagOfHandles: { args: ['handle', 'handle'], result: 'flag' },
    numberOfHandles: { args: ['handle', 'handle', 'outNumber'], result: 'outNumber' },
    outOfHandles: { args: ['handle', 'handle', 'out'], result: 'out' },
    parse: { args: ['bytes', 'flags', 'handle', 'out'], result: 'out' },
    parseFile: { args: ['string', 'flags', 'handle', 'out'], result: 'out' },
    save: { args: ['handle', 'string', 'flags', 'string'], result: 'status' },
    loadSchema: { args: ['bytes', 'out'], result: 'out' }
});

/*
 * Функции, доступные JavaScript, и их сигнатуры — тот же набор, что в
 * binding_table.cpp аддона Node.js: общий JS-слой (классы узлов, XPath,
 * схемы) работает с любым из двух бэкендов, не зная, какой под ним.
 *
 * The functions available to JavaScript and their signatures — the same set as in
 * binding_table.cpp of the Node.js addon: the common JS layer (node classes, XPath,
 * schemas) works with either of the two backends without knowing which one is underneath.
 */
const BINDINGS = Object.freeze([
    ['version', 'borrowed'], ['status_name', 'borrowedOfInt'], ['set_language', 'statusOfString'],
    ['language', 'borrowed'], ['languages', 'borrowed'], ['message_text', 'borrowedOfInt'],
    ['document_create', 'outOfString'], ['document_parse', 'parse'], ['document_parse_file', 'parseFile'],
    ['document_free', 'voidOfHandle'], ['document_clone', 'outOfHandle'], ['document_root', 'handleOfHandle'],
    ['document_element_by_id', 'handleOfHandleString'], ['document_save', 'save'],
    ['node_type', 'integerOfHandle'], ['node_name', 'ownedOfHandle'], ['node_text', 'ownedOfHandle'],
    ['node_to_string', 'ownedOfHandleFlags'], ['node_parent', 'handleOfHandle'],
    ['node_first_child', 'handleOfHandle'], ['node_last_child', 'handleOfHandle'],
    ['node_next_sibling', 'handleOfHandle'], ['node_previous_sibling', 'handleOfHandle'],
    ['node_owner_document', 'handleOfHandle'], ['node_child_count', 'countOfHandle'],
    ['node_child', 'handleOfHandleIndex'], ['node_set_text', 'statusOfHandleString'],
    ['node_remove', 'statusOfHandle'], ['node_copy_to', 'outOfHandles'], ['node_move_to', 'statusOfHandles'],
    ['node_append_element', 'handleOfHandleString'], ['node_append_text', 'handleOfHandleString'],
    ['node_append_comment', 'handleOfHandleString'], ['node_append_cdata', 'handleOfHandleString'],
    ['node_append_pi', 'handleOfHandleStrings'],
    ['element_get_attribute', 'ownedOfHandleString'], ['element_set_attribute', 'statusOfHandleStrings'],
    ['element_has_attribute', 'flagOfHandleString'], ['element_remove_attribute', 'flagOfHandleString'],
    ['element_attribute_count', 'countOfHandle'], ['element_attribute_name', 'ownedOfHandleIndex'],
    ['element_attribute_value', 'ownedOfHandleIndex'], ['element_local_name', 'ownedOfHandle'],
    ['element_namespace_uri', 'ownedOfHandle'],
    ['xpath_compile', 'outOfString'], ['xpath_free', 'voidOfHandle'],
    ['xpath_bind_namespace', 'statusOfHandleStrings'], ['xpath_set_string', 'statusOfHandleStrings'],
    ['xpath_set_number', 'statusOfHandleStringNumber'], ['xpath_select', 'outOfHandles'],
    ['xpath_string', 'ownedOfHandles'], ['xpath_number', 'numberOfHandles'], ['xpath_boolean', 'flagOfHandles'],
    ['nodeset_size', 'countOfHandle'], ['nodeset_node', 'handleOfHandleIndex'],
    ['nodeset_value', 'ownedOfHandleIndex'], ['nodeset_free', 'voidOfHandle'],
    ['schema_load', 'loadSchema'], ['schema_load_file', 'outOfString'], ['schema_free', 'voidOfHandle'],
    ['schema_validate', 'outOfHandles'], ['diagnostics_count', 'countOfHandle'],
    ['diagnostics_message', 'ownedOfHandleIndex'], ['diagnostics_line', 'countOfHandleIndex'],
    ['diagnostics_column', 'countOfHandleIndex'], ['diagnostics_free', 'voidOfHandle']
]);

module.exports = { SIGNATURES, BINDINGS };
