"""
Флаги и перечисления Си-интерфейса (значения совпадают с beresta_c.h).

IntFlag позволяет объединять флаги привычным оператором |:
    beresta.parse(xml, ParseFlags.DISCARD_COMMENTS | ParseFlags.VALIDATE_DTD)

Flags and enumerations of the C interface (the values match beresta_c.h).
IntFlag allows combining flags with the usual | operator:
beresta.parse(xml, ParseFlags.DISCARD_COMMENTS | ParseFlags.VALIDATE_DTD)
"""

from __future__ import annotations

from enum import IntEnum, IntFlag


class ParseFlags(IntFlag):
    """Флаги разбора BERESTA_PARSE_*. / Parsing flags BERESTA_PARSE_*."""

    DEFAULT = 0
    PRESERVE_WHITESPACE = 1 << 0
    DISCARD_COMMENTS = 1 << 1
    COALESCE_CDATA = 1 << 2
    VALIDATE_DTD = 1 << 3
    NO_NAMESPACES = 1 << 4
    XINCLUDE = 1 << 5
    FILE_RESOURCES = 1 << 6


class WriteFlags(IntFlag):
    """Флаги вывода BERESTA_WRITE_*. / Output flags BERESTA_WRITE_*."""

    DEFAULT = 0
    PRETTY = 1 << 0
    DECLARATION = 1 << 1


class NodeType(IntEnum):
    """Тип узла (beresta_node_kind). / The node type (beresta_node_kind)."""

    DOCUMENT = 0
    DOCUMENT_TYPE = 1
    ELEMENT = 2
    TEXT = 3
    CDATA_SECTION = 4
    COMMENT = 5
    PROCESSING_INSTRUCTION = 6
