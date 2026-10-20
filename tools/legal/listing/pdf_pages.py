"""Содержимое страниц: листинг (колонтитул, 50 строк кода, номер страницы), титульный лист
и страница простого текста (для реферата).

Формат страницы листинга: A4, поля 20 мм слева (под переплёт) и по 10–15 мм с остальных сторон,
моноширинный шрифт 10 пт (высота прописных больше 2 мм), 50 строк кода на странице. Внизу —
сквозной номер страницы депонируемых материалов и в скобках её номер в полном листинге,
как разрешает п. 29 Требований к документам заявки (приказ № 211).

The page content: the listing (the header, 50 code lines, the page number), the title page
and a plain text page (for the abstract).
The listing page format: A4, a 20 mm margin on the left (for binding) and 10–15 mm on the other
sides, a 10 pt monospaced font (the cap height is above 2 mm), 50 code lines per page. At the bottom
there is the running number of the deposit page and in brackets its number in the full listing,
as clause 29 of the Requirements for application documents (Order No. 211) allows.
"""

from __future__ import annotations

from .pdf_font import encode_text, text_width

# Размеры A4 и поля в пунктах (1 мм = 2,8346 пт) / The A4 size and margins in points (1 mm = 2.8346 pt)
PAGE = {"width": 595.28, "height": 841.89, "left": 56.69, "right": 28.35, "top": 42.52, "bottom": 42.52}


def page_geometry(font: dict, size: float, leading: float, lines: int) -> dict:
    """Геометрия страницы: кегль, интерлиньяж, строк на странице и сколько символов влезает в строку.
    The page geometry: the size, the leading, lines per page and how many characters fit in a line."""
    geometry = dict(PAGE)
    geometry.update({"size": size, "leading": leading, "lines": lines})
    usable = geometry["width"] - geometry["left"] - geometry["right"]
    geometry["columns"] = int(usable // text_width(font, "0", size))
    # Первая строка текста — под колонтитулом и линией / The first text line goes under the header and the rule
    geometry["first"] = geometry["height"] - geometry["top"] - 14 - leading
    return geometry


def _text(font: dict, x: float, y: float, size: float, value: str) -> str:
    """Оператор вывода строки в точке (x, y) кеглем size.
    The operator that shows a string at the point (x, y) with the size size."""
    return "BT /F1 %.2f Tf 1 0 0 1 %.2f %.2f Tm %s Tj ET\n" % (size, x, y, encode_text(font, value))


def _frame(font: dict, geometry: dict, header: tuple, footer: str) -> str:
    """Колонтитул (слева название, справа файл), линии и номер страницы внизу по центру.
    The header (the title on the left, the file on the right), the rules and the page number centered at the bottom."""
    right = geometry["width"] - geometry["right"]
    top = geometry["height"] - geometry["top"]
    ops = "0 g 0 G 0.5 w\n" + _text(font, geometry["left"], top - 8, 8, header[0])
    if header[1]:
        name = header[1][-55:]
        ops += _text(font, right - text_width(font, name, 8), top - 8, 8, name)
    ops += "%.2f %.2f m %.2f %.2f l S\n" % (geometry["left"], top - 12, right, top - 12)
    bottom = geometry["bottom"]
    ops += "%.2f %.2f m %.2f %.2f l S\n" % (geometry["left"], bottom + 20, right, bottom + 20)
    middle = (geometry["left"] + right - text_width(font, footer, 9)) / 2
    return ops + _text(font, middle, bottom + 6, 9, footer)


def body_page(font: dict, geometry: dict, frame: tuple, lines: list) -> bytes:
    """Страница листинга или текста: рамка (колонтитул, номер) и строки сверху вниз.
    frame = (название, файл, нижний колонтитул).
    A listing or text page: the frame (the header, the number) and the lines from top to bottom.
    frame = (title, file, footer)."""
    ops = _frame(font, geometry, (frame[0], frame[1]), frame[2])
    y = geometry["first"]
    for line in lines:
        if line:
            ops += _text(font, geometry["left"], y, geometry["size"], line)
        y -= geometry["leading"]
    return ops.encode("ascii")


def title_page(font: dict, geometry: dict, blocks: list) -> bytes:
    """Титульный лист: строки (текст, кегль, отступ сверху) по центру страницы.
    The title page: lines (text, size, space above) centered on the page."""
    ops = "0 g\n"
    y = geometry["height"] - 200
    for value, size, gap in blocks:
        y -= gap
        x = (geometry["width"] - text_width(font, value, size)) / 2
        ops += _text(font, max(x, geometry["left"]), y, size, value)
    return ops.encode("ascii")
