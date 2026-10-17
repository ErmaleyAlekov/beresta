"""Примесь: тип, имя, текст и сериализация узла."""

from __future__ import annotations

from ._native import LIB, encode
from .errors import check, take_text
from .flags import NodeType, WriteFlags


class NodeContentMixin:
    """Свойства содержимого узла."""

    @property
    def type(self) -> NodeType:
        """Тип узла (NodeType)."""
        code = LIB.beresta_node_type(self.handle)
        if code < 0:
            check(LIB.beresta_last_error_code())
        return NodeType(code)

    @property
    def name(self) -> str:
        """Имя по соглашению DOM: тег, цель PI, "#text", "#comment"..."""
        return take_text(LIB.beresta_node_name(self.handle))

    @property
    def text(self) -> str:
        """Текстовое содержимое (у элемента — весь текст потомков)."""
        return take_text(LIB.beresta_node_text(self.handle))

    @text.setter
    def text(self, value: str) -> None:
        """У элемента все дети заменяются одним текстовым узлом."""
        check(LIB.beresta_node_set_text(self.handle, encode(value)))

    def to_xml(self, flags: WriteFlags = WriteFlags.DEFAULT) -> str:
        """Сериализация узла с потомками (по умолчанию — без лишних пробелов)."""
        return take_text(LIB.beresta_node_to_string(self.handle, int(flags)))
