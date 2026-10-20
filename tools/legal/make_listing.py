#!/usr/bin/env python3
"""Листинг исходного текста «Берёсты» в PDF для депонирования в Роспатенте.

Собирает исходный текст программы (include/ и src/) в фиксированном порядке, раскладывает
по страницам A4 по 50 строк и выводит титульный лист, первые 25 и последние 25 страниц
(с номерами страниц в полном листинге в скобках). Режим --text превращает текстовый файл
(реферат) в PDF тем же шрифтом. Только стандартная библиотека Python 3.8+; шрифт —
моноширинный TrueType из системы (Consolas, Courier New, DejaVu Sans Mono) или --font.

Примеры:
    python tools/legal/make_listing.py --holder "[ФИО]" --authors "[ФИО]"
    python tools/legal/make_listing.py --text legal/referat.txt --output legal/referat.pdf

The Beresta source listing as a PDF for deposit with Rospatent.
It gathers the program source text (include/ and src/) in a fixed order, lays it out into A4 pages
of 50 lines and outputs the title page, the first 25 and the last 25 pages (with the page numbers
in the full listing in brackets). The --text mode turns a text file (the abstract) into a PDF with
the same font. The Python 3.8+ standard library only; the font is a monospaced system TrueType
(Consolas, Courier New, DejaVu Sans Mono) or --font.
"""

from __future__ import annotations

import argparse
import os
import sys
from datetime import datetime, timezone
from pathlib import Path

from listing import messages
from listing.errors import ListingError
from listing.layout import build_lines, paginate, select_pages
from listing.pdf_document import write_listing, write_text
from listing.pdf_metadata import find_icc
from listing.pdf_pages import page_geometry
from listing.sources import collect_files, read_version
from listing.ttf_reader import load_font

# Моноширинные шрифты с кириллицей на Windows, Linux и macOS (первый найденный)
# Monospaced fonts with Cyrillic on Windows, Linux and macOS (the first one found)
FONT_CANDIDATES = (
    "C:/Windows/Fonts/consola.ttf", "C:/Windows/Fonts/cour.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", "/usr/share/fonts/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/TTF/DejaVuSansMono.ttf", "/usr/share/fonts/dejavu-sans-mono-fonts/DejaVuSansMono.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
    "/System/Library/Fonts/Supplemental/Courier New.ttf", "/Library/Fonts/Courier New.ttf",
)

ROOT = Path(__file__).resolve().parents[2]


def parse_arguments() -> argparse.Namespace:
    """Параметры командной строки; заполнители ФИО — по умолчанию.
    The command-line parameters; the name placeholders are the defaults."""
    parser = argparse.ArgumentParser(description=messages.text("usage"))
    parser.add_argument("--root", default=str(ROOT))
    parser.add_argument("--output", default=str(ROOT / "legal" / "beresta_listing.pdf"))
    parser.add_argument("--head", type=int, default=25)
    parser.add_argument("--tail", type=int, default=25)
    parser.add_argument("--full", action="store_true")
    parser.add_argument("--font", default="")
    parser.add_argument("--icc", default="")
    parser.add_argument("--holder", default=messages.document_text("holder_placeholder"))
    parser.add_argument("--authors", default=messages.document_text("authors_placeholder"))
    parser.add_argument("--date", default="")
    parser.add_argument("--text", default="")
    parser.add_argument("--lang", default=os.environ.get("BERESTA_LANG", "ru"))
    return parser.parse_args()


def resolve_inputs(args: argparse.Namespace) -> dict:
    """Общие входные данные: шрифт, профиль ICC, дата документа (--date, SOURCE_DATE_EPOCH или сегодня).
    The common inputs: the font, the ICC profile, the document date (--date, SOURCE_DATE_EPOCH or today)."""
    candidates = [args.font] if args.font else list(FONT_CANDIDATES)
    found = [Path(c) for c in candidates if Path(c).is_file()]
    if not found:
        raise ListingError("no_font")
    if args.date:
        moment = datetime.strptime(args.date, "%Y-%m-%d").replace(tzinfo=timezone.utc)
    elif os.environ.get("SOURCE_DATE_EPOCH"):
        moment = datetime.fromtimestamp(int(os.environ["SOURCE_DATE_EPOCH"]), timezone.utc)
    else:
        # Только дата, без времени: повторный запуск в тот же день даёт тот же файл
        # The date only, without the time: a rerun on the same day gives the same file
        now = datetime.now(timezone.utc)
        moment = datetime(now.year, now.month, now.day, tzinfo=timezone.utc)
    icc = find_icc(args.icc)
    if icc is None:
        print(messages.text("no_icc"), file=sys.stderr)
    return {"font": load_font(found[0]), "moment": moment, "icc": icc, "output": Path(args.output)}


def run_listing(args: argparse.Namespace, context: dict) -> None:
    """Листинг: исходники -> строки -> страницы -> выбранные страницы -> PDF.
    The listing: sources -> lines -> pages -> the chosen pages -> PDF."""
    root = Path(args.root)
    context["version"] = read_version(root)
    columns = page_geometry(context["font"], 10, 14, 50)["columns"]
    lines, stats = build_lines(root, collect_files(root), columns)
    pages = paginate(lines, 50)
    head, tail = args.head, args.tail
    if args.full:
        head, tail = len(pages), 0
    context.update({"stats": stats, "total": len(pages), "head": head, "tail": tail,
                    "pages": select_pages(pages, head, tail), "holder": args.holder, "authors": args.authors,
                    "title": messages.document_text("header", context["version"])})
    count = write_listing(context)
    print(messages.text("stats", stats["files"], stats["lines"], stats["bytes"]))
    print(messages.text("done", context["output"], count, len(pages)))


def main() -> int:
    """Точка входа: листинг или (--text) текстовый документ; ошибки — сообщением без трассировки.
    The entry point: the listing or (--text) a text document; errors as a message without a trace."""
    # Вывод в UTF-8 на любой ОС: консоль Windows по умолчанию в кодовой странице 1251 или 866
    # Output in UTF-8 on any OS: the Windows console defaults to code page 1251 or 866
    for stream in (sys.stdout, sys.stderr):
        stream.reconfigure(encoding="utf-8", errors="replace")
    args = parse_arguments()
    messages.set_language(args.lang)
    try:
        context = resolve_inputs(args)
        context["output"].parent.mkdir(parents=True, exist_ok=True)
        if args.text:
            paragraphs = Path(args.text).read_text(encoding="utf-8").split("\n\n")
            context["title"] = paragraphs[0].strip()
            count = write_text(context, [" ".join(p.split()) for p in paragraphs])
            print(messages.text("done", context["output"], count, count))
        else:
            run_listing(args, context)
    except ListingError as error:
        print(error, file=sys.stderr)
        return 1
    if context["font"].get("missing"):
        print(messages.text("missing_glyphs", context["font"]["missing"]), file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
