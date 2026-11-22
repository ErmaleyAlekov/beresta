"""Печать HTML в PDF установленным браузером Chromium (Chrome, Edge, Chromium) в режиме без окна.

Своя вёрстка PDF для документов с таблицами и кодом заняла бы тысячи строк, а браузер
уже есть почти на любой машине и печатает по правилу @page из doc.css. Браузер — не
зависимость сборки библиотеки: он нужен только тому, кто готовит документы для реестра.

Printing HTML to PDF with an installed Chromium browser (Chrome, Edge, Chromium) in windowless mode.
An own PDF layout for documents with tables and code would take thousands of lines, while a browser
already exists on almost any machine and prints by the @page rule of doc.css. The browser is not a
dependency of the library build: only whoever prepares the registry documents needs it.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path

from .errors import DocsError

# Где искать браузер на Windows, macOS и Linux / Where to look for the browser on Windows, macOS and Linux
BROWSER_PATHS = (
    "C:/Program Files/Google/Chrome/Application/chrome.exe",
    "C:/Program Files (x86)/Google/Chrome/Application/chrome.exe",
    "C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe",
    "C:/Program Files/Microsoft/Edge/Application/msedge.exe",
    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
    "/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge",
    "/Applications/Chromium.app/Contents/MacOS/Chromium",
)
BROWSER_NAMES = ("chromium", "chromium-browser", "google-chrome", "google-chrome-stable", "microsoft-edge", "yandex-browser")
PRINT_TIMEOUT = 180


def find_browser(explicit: str) -> str:
    """Путь к браузеру: --browser, переменная BROWSER, известные пути, затем PATH.
    The browser path: --browser, the BROWSER variable, the known paths, then PATH."""
    for candidate in (explicit, os.environ.get("BROWSER", ""), *BROWSER_PATHS):
        if candidate and os.path.isfile(candidate) and os.access(candidate, os.X_OK):
            return str(Path(candidate).absolute())
    for name in BROWSER_NAMES:
        found = shutil.which(name)
        if found:
            return found
    raise DocsError("no_browser")


def print_pdf(browser: str, page: Path, output: Path) -> None:
    """Печать страницы в PDF без колонтитулов браузера (номер страницы ставит @page из CSS).
    Prints the page to PDF without the browser headers (the page number comes from @page in CSS)."""
    # Отдельный профиль и новый PDF исключают конфликт с открытым браузером и старым результатом.
    # An isolated profile and a fresh PDF avoid conflicts with the running browser and any old output.
    with tempfile.TemporaryDirectory(dir=output.parent.resolve()) as temporary:
        profile = Path(temporary) / "profile"
        staged_pdf = Path(temporary) / output.name
        command = [browser, "--headless=new", "--disable-gpu", "--no-first-run", "--no-pdf-header-footer",
                   "--user-data-dir=" + str(profile), "--print-to-pdf=" + str(staged_pdf),
                   page.resolve().as_uri()]
        try:
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=PRINT_TIMEOUT)
        except subprocess.TimeoutExpired as error:
            raise DocsError("browser_timeout", output, PRINT_TIMEOUT) from error
        except OSError as error:
            raise DocsError("browser_start_failed", browser, error) from error
        if result.returncode != 0 or not staged_pdf.is_file():
            raise DocsError("print_failed", output, result.returncode)
        staged_pdf.replace(output)
