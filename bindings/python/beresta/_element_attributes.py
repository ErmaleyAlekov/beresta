"""Примесь: атрибуты элемента."""

from __future__ import annotations

from typing import Dict, Optional

from ._native import LIB, encode
from .errors import check, take_text


class ElementAttributesMixin:
    """Чтение, запись и перебор атрибутов (порядок — как в документе)."""

    def get_attribute(self, name: str, default: Optional[str] = None) -> Optional[str]:
        """Значение атрибута или default, если атрибута нет."""
        value = take_text(LIB.beresta_element_get_attribute(self.handle, encode(name)))
        if value is None:
            return default
        return value

    def set_attribute(self, name: str, value: str) -> None:
        check(LIB.beresta_element_set_attribute(self.handle, encode(name), encode(value)))

    def has_attribute(self, name: str) -> bool:
        result = LIB.beresta_element_has_attribute(self.handle, encode(name))
        if result < 0:
            check(LIB.beresta_last_error_code())
        return result == 1

    def remove_attribute(self, name: str) -> bool:
        """True — атрибут был и удалён, False — его не было."""
        result = LIB.beresta_element_remove_attribute(self.handle, encode(name))
        if result < 0:
            check(LIB.beresta_last_error_code())
        return result == 1

    @property
    def attributes(self) -> Dict[str, str]:
        """Снимок всех атрибутов: {имя: значение} в порядке документа."""
        result = {}
        handle = self.handle
        for index in range(LIB.beresta_element_attribute_count(handle)):
            name = take_text(LIB.beresta_element_attribute_name(handle, index))
            result[name] = take_text(LIB.beresta_element_attribute_value(handle, index))
        return result
