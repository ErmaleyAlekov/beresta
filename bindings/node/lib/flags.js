'use strict';

/*
 * Флаги и перечисления Си-интерфейса (значения совпадают с beresta_c.h).
 * Флаги объединяются оператором |:
 *   beresta.parse(xml, { flags: ParseFlags.DISCARD_COMMENTS | ParseFlags.VALIDATE_DTD })
 * Объекты заморожены: опечатка в имени даёт undefined, а не тихую запись.
 *
 * Flags and enumerations of the C interface (the values match beresta_c.h).
 * Flags are combined with the | operator:
 *   beresta.parse(xml, { flags: ParseFlags.DISCARD_COMMENTS | ParseFlags.VALIDATE_DTD })
 * The objects are frozen: a typo in a name gives undefined, not a silent write.
 */

// Флаги разбора BERESTA_PARSE_* / Parsing flags BERESTA_PARSE_*
const ParseFlags = Object.freeze({
    DEFAULT: 0,
    PRESERVE_WHITESPACE: 1 << 0,
    DISCARD_COMMENTS: 1 << 1,
    COALESCE_CDATA: 1 << 2,
    VALIDATE_DTD: 1 << 3,
    NO_NAMESPACES: 1 << 4,
    XINCLUDE: 1 << 5,
    FILE_RESOURCES: 1 << 6
});

// Флаги вывода BERESTA_WRITE_*: по умолчанию — ни одного лишнего пробела
// Output flags BERESTA_WRITE_*: by default — not a single extra space
const WriteFlags = Object.freeze({
    DEFAULT: 0,
    PRETTY: 1 << 0,
    DECLARATION: 1 << 1
});

// Тип узла (beresta_node_kind) / The node type (beresta_node_kind)
const NodeType = Object.freeze({
    DOCUMENT: 0,
    DOCUMENT_TYPE: 1,
    ELEMENT: 2,
    TEXT: 3,
    CDATA_SECTION: 4,
    COMMENT: 5,
    PROCESSING_INSTRUCTION: 6
});

module.exports = { ParseFlags, WriteFlags, NodeType };
