"""Проверяет код выхода демонстрации после обычной ошибки C API.
Checks the demo exit status after an ordinary C API error.
"""

import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parent
ROOT = TOOLS.parents[1]
COMPILER = shutil.which(os.environ.get("BERESTA_DEMO_CC", "gcc"))
LIBRARY_NAME = {"win32": "beresta.dll", "darwin": "libberesta.dylib"}.get(
    sys.platform, "libberesta.so",
)
LIBRARY = Path(os.environ.get("BERESTA_DEMO_LIBRARY", ROOT / "bin" / LIBRARY_NAME)).resolve()
BUILD_REQUIREMENTS = "Нужны GCC и библиотека «Берёста» / Requires GCC and the Beresta library"


@unittest.skipUnless(COMPILER and LIBRARY.is_file(), BUILD_REQUIREMENTS)
class CapiDemoTests(unittest.TestCase):
    """Тест запускает настоящий пример с подменой двух аргументов C API.
    The test runs the real demo with two injectable C API arguments.
    """

    def test_error_stops_demonstration(self):
        """Проверяет обычный запуск и оба пути ошибки.
        Checks the normal run and both error paths.
        """
        with tempfile.TemporaryDirectory() as directory:
            binary = Path(directory) / "capi_demo.exe"
            self.build_demo(binary)
            normal = self.run_demo(binary, "")
            self.assertEqual(normal.returncode, 0, normal.stderr)
            self.assertIn("ожидаемая ошибка", normal.stdout)
            for operation, diagnostic in (("xpath", "привязка XPath"), ("attribute", "построение документа")):
                with self.subTest(operation=operation):
                    failed = self.run_demo(binary, operation)
                    self.assertEqual(failed.returncode, 1, failed.stderr)
                    self.assertIn(f"{diagnostic}: [INVALID_ARGUMENT]", failed.stderr)
                    self.assertNotIn("ожидаемая ошибка", failed.stdout)

    @staticmethod
    def build_demo(binary):
        """Собирает только демонстрационный файл и заглушки.
        Builds only the demo source and the stubs.
        """
        process = subprocess.run(
            [COMPILER, "-std=c99", "-I", str(ROOT / "include"),
             str(TOOLS / "capi_demo_failure.c"), str(LIBRARY), "-o", str(binary)],
            capture_output=True, timeout=30,
        )
        if process.returncode != 0:
            raise AssertionError(process.stderr.decode("utf-8", errors="replace"))

    @staticmethod
    def run_demo(binary, operation):
        """Настраивает поиск библиотеки и выбирает подменяемый вызов.
        Configures library discovery and selects the call to override.
        """
        environment = dict(os.environ, BERESTA_DEMO_FAILURE=operation)
        library_search = {"win32": "PATH", "darwin": "DYLD_LIBRARY_PATH"}.get(
            sys.platform, "LD_LIBRARY_PATH",
        )
        environment[library_search] = os.pathsep.join(
            [str(LIBRARY.parent), environment.get(library_search, "")],
        )
        if sys.platform == "win32":
            shutil.copyfile(LIBRARY, binary.parent / LIBRARY.name)
        return subprocess.run(
            [str(binary)], capture_output=True, encoding="utf-8", timeout=30, env=environment,
        )


if __name__ == "__main__":
    unittest.main()
