"""
XPath — скомпилированное выражение XPath 1.0.

Компилируется один раз, вычисляется сколько угодно раз для разных узлов
и документов. Префиксы и переменные задаются в конструкторе или позже.

XPath — a compiled XPath 1.0 expression.
It is compiled once and evaluated any number of times for different nodes
and documents. Prefixes and variables are set in the constructor or later.
"""

from __future__ import annotations

import weakref
from typing import Dict, Optional

from ._native import LIB, encode, out_handle
from ._wrapping import wrap_node
from ._xpath_scalars import XPathScalarsMixin
from .errors import check, take_text


class XPath(XPathScalarsMixin):
    """Пример: XPath("//s:item[@code = $c]", {"s": "urn:shop"}, {"c": "B2"}).
    Example: XPath("//s:item[@code = $c]", {"s": "urn:shop"}, {"c": "B2"})."""

    def __init__(self, expression: str, namespaces: Optional[Dict[str, str]] = None,
                 variables: Optional[Dict[str, object]] = None) -> None:
        result = out_handle()
        check(LIB.beresta_xpath_compile(encode(expression), result))
        self.handle = result.value
        self._finalizer = weakref.finalize(self, LIB.beresta_xpath_free, self.handle)
        for prefix, uri in (namespaces or {}).items():
            self.bind(prefix, uri)
        for name, value in (variables or {}).items():
            self.set_variable(name, value)

    def bind(self, prefix: str, uri: str) -> "XPath":
        """Префикс для имён p:name в выражении. / A prefix for p:name names in the expression."""
        check(LIB.beresta_xpath_bind_namespace(self.handle, encode(prefix), encode(uri)))
        return self

    def set_variable(self, name: str, value) -> "XPath":
        """Переменная $name: число (int/float) или строка. / The variable $name: a number (int/float) or a string."""
        if isinstance(value, (int, float)) and not isinstance(value, bool):
            check(LIB.beresta_xpath_set_number(self.handle, encode(name), float(value)))
        else:
            check(LIB.beresta_xpath_set_string(self.handle, encode(name), encode(str(value))))
        return self

    def select(self, node) -> list:
        """
        Множество узлов в порядке документа. Атрибуты в DOM «Берёсты» —
        не узлы, поэтому вместо них в списке их строковые значения (str).

        Nodes in document order. Attributes in the Beresta DOM are
        not nodes, so the list contains their string values (str) instead.
        """
        result = out_handle()
        check(LIB.beresta_xpath_select(self.handle, node.handle, result))
        try:
            items = []
            for index in range(LIB.beresta_nodeset_size(result)):
                pointer = LIB.beresta_nodeset_node(result, index)
                if pointer is None:
                    items.append(take_text(LIB.beresta_nodeset_value(result, index)))
                else:
                    items.append(wrap_node(pointer, node.document))
            return items
        finally:
            LIB.beresta_nodeset_free(result)
