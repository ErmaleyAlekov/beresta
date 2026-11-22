"""Регрессии разбора заголовка Си-интерфейса.
Regressions for parsing the C API header."""

from pathlib import Path
import re
import unittest

from tools.registry.docgen.capi_parse import parse_codes, parse_flags, parse_functions


class CapiTests(unittest.TestCase):
    """Проверки экспортов, кодов состояния и флагов.
    Checks for exports, status codes, and flags."""

    def test_identifiers_can_contain_digits(self):
        """Цифры сохраняются в именах флагов и кодов.
        Digits are preserved in flag and status code names."""
        lines = [
            "typedef enum beresta_status {",
            "    BERESTA_ERR_UTF8 = 9, /* Ошибка / Invalid text */",
            "} beresta_status;",
            "#define BERESTA_PARSE_1_1 (1u << 9) /* Разбор / Parse flag */",
        ]
        self.assertEqual(parse_codes(lines), [("BERESTA_ERR_UTF8", "9", "Ошибка")])
        self.assertEqual(parse_flags(lines), [("BERESTA_PARSE_1_1", "(1u << 9)", "Разбор")])

    def test_exported_data_is_skipped_without_leaking_comment(self):
        """Переменная пропускается, и её комментарий не относится к функции.
        Exported data is skipped and its comment does not describe the function."""
        lines = [
            "/* ====",
            "**  API",
            "*/",
            "/* Данные */",
            "BERESTA_API extern int counter;",
            "/* Функция */",
            "BERESTA_API int beresta_probe(void);",
        ]
        self.assertEqual(parse_functions(lines), [("API", [("int beresta_probe(void)", "beresta_probe", "Функция")])])

    def test_current_header_functions_appear_in_order(self):
        """Список функций справочника совпадает с экспортами заголовка.
        The reference function list matches the exports in the header."""
        header = Path(__file__).resolve().parents[2] / "include" / "capi" / "beresta_c.h"
        source = header.read_text(encoding="utf-8")
        expected = re.findall(r"^BERESTA_API\s+[^;]*?\b(beresta_\w+)\s*\(", source, re.MULTILINE)
        actual = [name for _, group in parse_functions(source.splitlines()) for _, name, _ in group]
        self.assertTrue(expected)
        self.assertEqual(actual, expected)
