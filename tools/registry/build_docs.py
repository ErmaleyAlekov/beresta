#!/usr/bin/env python3
"""Сборка документации для реестра российского ПО (Минцифры): docs/registry/*.md -> site/docs/*.html и *.pdf.

Исходники документов — Markdown в docs/registry/. Сборщик переводит их в HTML своим
разборщиком подмножества Markdown (только стандартная библиотека Python 3.8+), подставляет
справочник Си-интерфейса из include/capi/beresta_c.h вместо строки <!-- capi-reference -->
и печатает каждую страницу в PDF браузером Chrome / Edge / Chromium без окна.

Примеры:
    python tools/registry/build_docs.py
    python tools/registry/build_docs.py --html-only
    python tools/registry/build_docs.py --browser /usr/bin/chromium

Building the documentation for the Russian software registry (the Ministry of Digital Development):
docs/registry/*.md -> site/docs/*.html and *.pdf. The document sources are Markdown in docs/registry/.
The builder turns them into HTML with its own parser of a Markdown subset (the Python 3.8+ standard
library only), puts the C interface reference from include/capi/beresta_c.h instead of the
<!-- capi-reference --> line and prints every page to PDF with Chrome / Edge / Chromium without a window.
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from pathlib import Path

from docgen import messages
from docgen.browser_pdf import find_browser, print_pdf
from docgen.capi_reference import reference_markdown
from docgen.errors import DocsError
from docgen.html_page import build_page
from docgen.md_blocks import render_markdown

ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "include" / "capi" / "beresta_c.h"
REFERENCE_MARK = "<!-- capi-reference -->"


def parse_arguments() -> argparse.Namespace:
    """Параметры командной строки.
    The command-line parameters."""
    parser = argparse.ArgumentParser(description=messages.text("usage"))
    parser.add_argument("--source", default=str(ROOT / "docs" / "registry"))
    parser.add_argument("--output", default=str(ROOT / "site" / "docs"))
    parser.add_argument("--browser", default="")
    parser.add_argument("--html-only", action="store_true")
    parser.add_argument("--lang", default=os.environ.get("BERESTA_LANG", "ru"))
    return parser.parse_args()


def library_version() -> str:
    """Версия из BERESTA_VERSION_STRING (та же, что в CMakeLists.txt).
    The version from BERESTA_VERSION_STRING (the same as in CMakeLists.txt)."""
    match = re.search(r'BERESTA_VERSION_STRING\s+"([^"]+)"', HEADER.read_text(encoding="utf-8"))
    if match is None:
        raise DocsError("no_sources", HEADER)
    return match.group(1)


def build_one(source: Path, output: Path, version: str, browser: str) -> None:
    """Один документ: Markdown (+ справочник API) -> HTML, затем (если есть браузер) PDF.
    One document: Markdown (+ the API reference) -> HTML, then (if there is a browser) PDF."""
    text = source.read_text(encoding="utf-8")
    if REFERENCE_MARK in text:
        text = text.replace(REFERENCE_MARK, reference_markdown(HEADER))
    css = (Path(__file__).parent / "doc.css").read_text(encoding="utf-8")
    page = output / (source.stem + ".html")
    page.write_text(build_page(render_markdown(text), version, css), encoding="utf-8", newline="\n")
    print(messages.text("written", page))
    if browser:
        pdf = output / (source.stem + ".pdf")
        print_pdf(browser, page, pdf)
        print(messages.text("written", pdf))


def main() -> int:
    """Точка входа: все *.md каталога-источника по алфавиту; ошибки — сообщением.
    The entry point: all *.md of the source directory alphabetically; errors as a message."""
    for stream in (sys.stdout, sys.stderr):
        stream.reconfigure(encoding="utf-8", errors="replace")
    args = parse_arguments()
    messages.set_language(args.lang)
    try:
        sources = sorted(Path(args.source).glob("*.md"))
        if not sources:
            raise DocsError("no_sources", args.source)
        browser = ""
        if not args.html_only:
            browser = find_browser(args.browser)
        output = Path(args.output)
        output.mkdir(parents=True, exist_ok=True)
        for source in sources:
            build_one(source, output, library_version(), browser)
    except DocsError as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
