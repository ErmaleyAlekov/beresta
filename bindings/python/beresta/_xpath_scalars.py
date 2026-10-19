"""Примесь XPath: результат как строка, число, логическое значение или
список строковых значений (select_values).
XPath mixin: the result as a string, a number, a boolean or a list of string
values (select_values)."""

from __future__ import annotations

from ctypes import byref, c_double

from ._native import LIB, out_handle
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

    def select_values(self, node) -> list:
        """
        Строковые значения найденных узлов (текст элемента, значение
        атрибута) без создания объектов-обёртки — самый быстрый способ
        читать данные из Python: значения собираются внутри ядра, а
        границу ctypes пересекает только по одной строке на узел.

        The string values of the found nodes (the text of an element, the value
        of an attribute) without creating wrapper objects — the fastest way to
        read data from Python: the values are assembled inside the core, and only
        one string per node crosses the ctypes boundary.
        """
        result = out_handle()
        check(LIB.beresta_xpath_select(self.handle, node.handle, result))
        try:
            return [take_text(LIB.beresta_nodeset_value(result, index))
                    for index in range(LIB.beresta_nodeset_size(result))]
        finally:
            LIB.beresta_nodeset_free(result)
