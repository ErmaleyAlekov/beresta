"""Element — элемент: узел с именем, атрибутами и пространством имён.
Element — an element: a node with a name, attributes and a namespace."""

from __future__ import annotations

from typing import Optional

from ._element_attributes import ElementAttributesMixin
from ._native import LIB
from .errors import take_text
from .node import Node


class Element(Node, ElementAttributesMixin):
    """Элемент XML. / An XML element."""

    @property
    def local_name(self) -> str:
        """Имя без префикса: у <s:item> — "item". / The name without a prefix: "item" for <s:item>."""
        return take_text(LIB.beresta_element_local_name(self.handle))

    @property
    def namespace_uri(self) -> Optional[str]:
        """URI пространства имён или None, если элемент вне пространств.
        The namespace URI or None if the element is in no namespace."""
        return take_text(LIB.beresta_element_namespace_uri(self.handle))

    def __repr__(self) -> str:
        return "<Element " + self.name + ">"
