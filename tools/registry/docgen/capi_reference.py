"""Справочник Си-интерфейса в виде Markdown: коды состояния, флаги и все функции по группам.
Подставляется в руководство по эксплуатации вместо строки <!-- capi-reference -->.

The C interface reference as Markdown: status codes, flags and all functions by groups.
It replaces the <!-- capi-reference --> line in the operation manual.
"""

from __future__ import annotations

from pathlib import Path

from .capi_parse import parse_codes, parse_flags, parse_functions

# Функции без собственного комментария в заголовке (смысл ясен из имени) — краткие описания
# Functions without their own comment in the header (the name says it all) — short descriptions
FALLBACK = {
    "beresta_node_parent": "Родительский узел или NULL у корня дерева",
    "beresta_node_first_child": "Первый дочерний узел или NULL",
    "beresta_node_last_child": "Последний дочерний узел или NULL",
    "beresta_node_next_sibling": "Следующий соседний узел или NULL",
    "beresta_node_previous_sibling": "Предыдущий соседний узел или NULL",
    "beresta_element_set_attribute": "Добавление или замена атрибута (позиция прежнего атрибута сохраняется)",
    "beresta_xpath_free": "Освобождение скомпилированного выражения (NULL допустим)",
    "beresta_schema_free": "Освобождение схемы (NULL допустим)",
    "beresta_nodeset_free": "Освобождение набора узлов (NULL допустим)",
    "beresta_diagnostics_free": "Освобождение списка нарушений (NULL допустим)",
    "beresta_schema_validate_file": "Потоковая проверка файла без построения дерева (см. beresta_schema_validate_bytes)",
}

# Значения без комментария / Values without a comment
CODE_FALLBACK = {"BERESTA_OK": "успех", "BERESTA_PARSE_DEFAULT": "настройки по умолчанию"}


def _table(title: str, rows: list) -> list:
    """Таблица «Имя | Значение | Описание» с заголовком третьего уровня.
    A "Name | Value | Description" table with a third-level heading."""
    lines = ["### " + title, "", "| Имя | Значение | Описание |", "|---|---|---|"]
    for name, value, description in rows:
        text = description or CODE_FALLBACK.get(name, "")
        lines.append("| `%s` | `%s` | %s |" % (name, value, text.replace("|", "\\|")))
    lines.append("")
    return lines


def _functions(groups: list) -> list:
    """Функции по группам: сигнатура кодом и описание из комментария (или из FALLBACK).
    The functions by groups: the signature as code and the description from the comment (or FALLBACK)."""
    lines = []
    for title, functions in groups:
        lines += ["### " + title, ""]
        for signature, name, description in functions:
            text = FALLBACK.get(name) or description
            lines.append("- `%s` — %s" % (signature, text))
        lines.append("")
    return lines


def reference_markdown(header: Path) -> str:
    """Весь справочник: число функций, коды, флаги, функции по группам.
    The whole reference: the function count, the codes, the flags, the functions by groups."""
    lines = header.read_text(encoding="utf-8").split("\n")
    groups = parse_functions(lines)
    count = sum(len(functions) for _title, functions in groups)
    result = ["Справочник построен из заголовка `include/capi/beresta_c.h` (функций: %d)." % count, ""]
    result += _table("Коды состояния", parse_codes(lines))
    result += _table("Флаги разбора и вывода", parse_flags(lines))
    result += _functions(groups)
    return "\n".join(result)
