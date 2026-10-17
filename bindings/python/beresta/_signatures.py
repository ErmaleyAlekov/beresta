"""
Сигнатуры функций Си-интерфейса для ctypes (зеркало include/capi/beresta_c.h).

ctypes по умолчанию считает, что функция возвращает int и принимает int:
на 64-битной платформе указатель обрезался бы до 32 бит. Поэтому для
КАЖДОЙ функции явно задаются restype и argtypes.

Возвращаемые строки char* объявлены как c_void_p, а не c_char_p: c_char_p
сразу копирует строку в bytes и теряет указатель — тогда его нельзя
передать в beresta_string_free, и память утекает.
"""

from __future__ import annotations

from ctypes import POINTER, c_char_p, c_double, c_int, c_int64, c_uint32, c_void_p

_HANDLE = c_void_p
_OUT_HANDLE = POINTER(c_void_p)
_OWNED_TEXT = c_void_p

# (имя, тип результата, типы аргументов)
SIGNATURES = (
    # ---- Библиотека и ошибки ----
    ("beresta_version", c_char_p, ()),
    ("beresta_last_error_code", c_int, ()),
    ("beresta_last_error_message", c_char_p, ()),
    ("beresta_status_name", c_char_p, (c_int,)),
    ("beresta_string_free", None, (c_void_p,)),
    # ---- Документы ----
    ("beresta_document_create", c_int, (c_char_p, _OUT_HANDLE)),
    ("beresta_document_parse", c_int, (c_char_p, c_int64, c_uint32, _HANDLE, _OUT_HANDLE)),
    ("beresta_document_parse_file", c_int, (c_char_p, c_uint32, _HANDLE, _OUT_HANDLE)),
    ("beresta_document_free", None, (_HANDLE,)),
    ("beresta_document_clone", c_int, (_HANDLE, _OUT_HANDLE)),
    ("beresta_document_root", _HANDLE, (_HANDLE,)),
    ("beresta_document_element_by_id", _HANDLE, (_HANDLE, c_char_p)),
    ("beresta_document_save", c_int, (_HANDLE, c_char_p, c_uint32, c_char_p)),
    # ---- Узлы: свойства и навигация ----
    ("beresta_node_type", c_int, (_HANDLE,)),
    ("beresta_node_name", _OWNED_TEXT, (_HANDLE,)),
    ("beresta_node_text", _OWNED_TEXT, (_HANDLE,)),
    ("beresta_node_to_string", _OWNED_TEXT, (_HANDLE, c_uint32)),
    ("beresta_node_parent", _HANDLE, (_HANDLE,)),
    ("beresta_node_first_child", _HANDLE, (_HANDLE,)),
    ("beresta_node_last_child", _HANDLE, (_HANDLE,)),
    ("beresta_node_next_sibling", _HANDLE, (_HANDLE,)),
    ("beresta_node_previous_sibling", _HANDLE, (_HANDLE,)),
    ("beresta_node_owner_document", _HANDLE, (_HANDLE,)),
    ("beresta_node_child_count", c_int64, (_HANDLE,)),
    ("beresta_node_child", _HANDLE, (_HANDLE, c_int64)),
    # ---- Узлы: изменение дерева ----
    ("beresta_node_set_text", c_int, (_HANDLE, c_char_p)),
    ("beresta_node_remove", c_int, (_HANDLE,)),
    ("beresta_node_copy_to", c_int, (_HANDLE, _HANDLE, _OUT_HANDLE)),
    ("beresta_node_append_element", _HANDLE, (_HANDLE, c_char_p)),
    ("beresta_node_append_text", _HANDLE, (_HANDLE, c_char_p)),
    ("beresta_node_append_comment", _HANDLE, (_HANDLE, c_char_p)),
    ("beresta_node_append_cdata", _HANDLE, (_HANDLE, c_char_p)),
    ("beresta_node_append_pi", _HANDLE, (_HANDLE, c_char_p, c_char_p)),
    # ---- Элементы и атрибуты ----
    ("beresta_element_get_attribute", _OWNED_TEXT, (_HANDLE, c_char_p)),
    ("beresta_element_set_attribute", c_int, (_HANDLE, c_char_p, c_char_p)),
    ("beresta_element_has_attribute", c_int, (_HANDLE, c_char_p)),
    ("beresta_element_remove_attribute", c_int, (_HANDLE, c_char_p)),
    ("beresta_element_attribute_count", c_int64, (_HANDLE,)),
    ("beresta_element_attribute_name", _OWNED_TEXT, (_HANDLE, c_int64)),
    ("beresta_element_attribute_value", _OWNED_TEXT, (_HANDLE, c_int64)),
    ("beresta_element_local_name", _OWNED_TEXT, (_HANDLE,)),
    ("beresta_element_namespace_uri", _OWNED_TEXT, (_HANDLE,)),
    # ---- XPath ----
    ("beresta_xpath_compile", c_int, (c_char_p, _OUT_HANDLE)),
    ("beresta_xpath_free", None, (_HANDLE,)),
    ("beresta_xpath_bind_namespace", c_int, (_HANDLE, c_char_p, c_char_p)),
    ("beresta_xpath_set_string", c_int, (_HANDLE, c_char_p, c_char_p)),
    ("beresta_xpath_set_number", c_int, (_HANDLE, c_char_p, c_double)),
    ("beresta_xpath_select", c_int, (_HANDLE, _HANDLE, _OUT_HANDLE)),
    ("beresta_xpath_string", _OWNED_TEXT, (_HANDLE, _HANDLE)),
    ("beresta_xpath_number", c_int, (_HANDLE, _HANDLE, POINTER(c_double))),
    ("beresta_xpath_boolean", c_int, (_HANDLE, _HANDLE)),
    ("beresta_nodeset_size", c_int64, (_HANDLE,)),
    ("beresta_nodeset_node", _HANDLE, (_HANDLE, c_int64)),
    ("beresta_nodeset_value", _OWNED_TEXT, (_HANDLE, c_int64)),
    ("beresta_nodeset_free", None, (_HANDLE,)),
    # ---- XML Schema ----
    ("beresta_schema_load", c_int, (c_char_p, c_int64, _OUT_HANDLE)),
    ("beresta_schema_load_file", c_int, (c_char_p, _OUT_HANDLE)),
    ("beresta_schema_free", None, (_HANDLE,)),
    ("beresta_schema_validate", c_int, (_HANDLE, _HANDLE, _OUT_HANDLE)),
    ("beresta_diagnostics_count", c_int64, (_HANDLE,)),
    ("beresta_diagnostics_message", _OWNED_TEXT, (_HANDLE, c_int64)),
    ("beresta_diagnostics_line", c_int64, (_HANDLE, c_int64)),
    ("beresta_diagnostics_column", c_int64, (_HANDLE, c_int64)),
    ("beresta_diagnostics_free", None, (_HANDLE,)),
)


def bind_signatures(library) -> None:
    """Назначение restype/argtypes всем функциям загруженной библиотеки."""
    for name, result_type, argument_types in SIGNATURES:
        function = getattr(library, name)
        function.restype = result_type
        function.argtypes = list(argument_types)
