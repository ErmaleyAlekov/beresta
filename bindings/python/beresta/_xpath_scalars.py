"""Примесь XPath: результат как строка, число или логическое значение.
XPath mixin: the result as a string, a number or a boolean."""

from __future__ import annotations

from ctypes import byref, c_double

from ._native import LIB
from .errors import check, take_text


class XPathScalarsMixin:
    """Преобразования по правилам функций string(), number(), boolean().
    Conversions by the rules of the string(), number(), boolean() functions."""

    def string(self, node) -> str:
        return take_text(LIB.beresta_xpath_string(self.handle, node.handle))

    def number(self, node) -> float:
        """NaN — не ошибка, а законный результат (например, number('abc')).
        NaN is not an error but a legitimate result (for example, number('abc'))."""
        value = c_double()
        check(LIB.beresta_xpath_number(self.handle, node.handle, byref(value)))
        return value.value

    def boolean(self, node) -> bool:
        result = LIB.beresta_xpath_boolean(self.handle, node.handle)
        if result < 0:
            check(LIB.beresta_last_error_code())
        return result == 1
