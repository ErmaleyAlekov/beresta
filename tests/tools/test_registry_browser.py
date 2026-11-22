"""Регрессии запуска браузера из генератора документов реестра.
Regressions for launching a browser from the registry documentation generator."""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from tools.registry.docgen import browser_pdf
from tools.registry.docgen.errors import DocsError


class BrowserTests(unittest.TestCase):
    """Проверки доступности браузера и ошибок печати.
    Checks for browser availability and printing errors."""

    def test_non_executable_candidate_is_skipped(self):
        """Обычный файл нельзя использовать как браузер без разрешения на запуск.
        A regular file needs execute permission to be used as a browser."""
        with tempfile.TemporaryDirectory() as temporary:
            blocked = Path(temporary) / "browser"
            blocked.touch()
            with patch.dict(os.environ, {"BROWSER": str(blocked)}), patch.object(browser_pdf, "BROWSER_PATHS", ()):
                with patch.object(browser_pdf.shutil, "which", return_value=None):
                    with patch.object(browser_pdf.os, "access", return_value=False) as access:
                        with self.assertRaises(DocsError):
                            browser_pdf.find_browser("")
                        access.assert_called_once_with(str(blocked), os.X_OK)

    def test_launch_exceptions_become_docs_errors(self):
        """Сбой запуска и тайм-аут обрабатываются как ошибки документов.
        Startup failures and timeouts are handled as document errors."""
        failures = [PermissionError(), OSError(), subprocess.TimeoutExpired(["browser"], 180)]
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            for failure in failures:
                with self.subTest(failure=type(failure)):
                    with patch.object(browser_pdf.subprocess, "run", side_effect=failure):
                        with self.assertRaises(DocsError) as captured:
                            browser_pdf.print_pdf("browser", root / "page.html", root / "out.pdf")
                    self.assertIs(captured.exception.__cause__, failure)
                    self.assertEqual(list(root.iterdir()), [])

    def test_stale_pdf_does_not_hide_failed_print(self):
        """Старый PDF остаётся на месте при отсутствии нового вывода.
        An old PDF remains untouched when printing yields no output."""
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            output = root / "out.pdf"
            output.write_bytes(b"old PDF")
            with patch.object(browser_pdf.subprocess, "run", return_value=subprocess.CompletedProcess([], 0)):
                with self.assertRaises(DocsError):
                    browser_pdf.print_pdf("browser", root / "page.html", output)
            self.assertEqual(output.read_bytes(), b"old PDF")
            self.assertEqual(list(root.iterdir()), [output])
