"""Каталог сообщений генератора листинга: тексты для пользователя на русском и английском.
Код ссылается на сообщение по идентификатору, а не пишет текст литералом. Тексты самого
документа (титульный лист, колонтитулы) — тоже здесь: их язык — русский, как требует Роспатент.

The message catalog of the listing generator: user-facing texts in Russian and English.
The code refers to a message by its identifier instead of writing the text as a literal.
The texts of the document itself (the title page, the running headers) are here too: their
language is Russian, as Rospatent requires.
"""

from __future__ import annotations

# Сообщения командной строки: идентификатор -> {язык: шаблон}; {0}, {1} — места аргументов
# Command-line messages: identifier -> {language: template}; {0}, {1} mark the arguments
CATALOG = {
    "usage": {
        "ru": "Листинг исходного текста «Берёсты» в PDF для депонирования в Роспатенте",
        "en": "The Beresta source listing as a PDF for deposit with Rospatent",
    },
    "no_font": {
        "ru": "Не найден моноширинный шрифт TrueType: укажите его через --font",
        "en": "No monospaced TrueType font is found: pass one with --font",
    },
    "bad_font": {
        "ru": "Шрифт {0} не подходит: {1}",
        "en": "The font {0} does not fit: {1}",
    },
    "font_restricted": {
        "ru": "лицензия шрифта запрещает встраивание",
        "en": "the font license forbids embedding",
    },
    "font_no_cmap": {
        "ru": "нет таблицы cmap формата 4 для Unicode",
        "en": "there is no format 4 cmap table for Unicode",
    },
    "font_no_table": {
        "ru": "нет обязательной таблицы {0}",
        "en": "the required table {0} is missing",
    },
    "no_sources": {
        "ru": "В {0} не найдено ни одного исходного файла",
        "en": "No source file is found in {0}",
    },
    "no_version": {
        "ru": "Не удалось прочитать BERESTA_VERSION_STRING из {0}",
        "en": "Cannot read BERESTA_VERSION_STRING from {0}",
    },
    "missing_glyphs": {
        "ru": "Символов без глифа в шрифте: {0} (заменены на «?»)",
        "en": "Characters without a glyph in the font: {0} (replaced with \"?\")",
    },
    "no_icc": {
        "ru": "Профиль sRGB не найден: файл без отметки PDF/A (укажите --icc)",
        "en": "No sRGB profile is found: the file has no PDF/A mark (pass --icc)",
    },
    "done": {
        "ru": "Готово: {0} — страниц в файле {1}, страниц листинга {2}",
        "en": "Done: {0} — pages in the file {1}, listing pages {2}",
    },
    "stats": {
        "ru": "Исходных файлов: {0}, строк: {1}, байт: {2}",
        "en": "Source files: {0}, lines: {1}, bytes: {2}",
    },
}

# Тексты документа (русские): колонтитул, нижний колонтитул, титульный лист
# The document texts (Russian): the header, the footer, the title page
DOCUMENT_TEXTS = {
    "header": "XML-библиотека «Берёста» {0} — исходный текст",
    "footer": "Стр. {0} ({1} из {2})",
    "text_footer": "Стр. {0} из {1}",
    "title_kind": "ПРОГРАММА ДЛЯ ЭВМ",
    "title_name": "XML-библиотека «Берёста» (beresta)",
    "title_version": "Версия {0}",
    "title_listing": "Исходный текст (фрагменты)",
    "title_holder": "Правообладатель: {0}",
    "title_authors": "Автор(ы): {0}",
    "title_language": "Язык программирования: C++17",
    "title_pages": "Представлены страницы 1–{0} и {1}–{2} листинга из {2}.",
    "title_pages_all": "Представлен полный листинг: {0} стр.",
    "title_numbers": "В скобках на каждой странице — её номер в полном листинге.",
    "title_volume": "Объём исходного текста: {0} файлов, {1} строк, {2} байт.",
    "title_year": "{0}",
    "holder_placeholder": "[ФИО правообладателя]",
    "authors_placeholder": "[ФИО автора (авторов)]",
    "file_mark": "// ===== Файл: {0} =====",
}

# Текущий язык сообщений командной строки
# The current language of the command-line messages
_STATE = {"language": "ru"}


def set_language(code: str) -> None:
    """Выбор языка сообщений: неизвестный код оставляет русский.
    Chooses the message language: an unknown code keeps Russian."""
    if code in ("ru", "en"):
        _STATE["language"] = code
    else:
        _STATE["language"] = "ru"


def text(identifier: str, *arguments: object) -> str:
    """Сообщение командной строки на текущем языке с подставленными аргументами.
    A command-line message in the current language with the arguments substituted."""
    template = CATALOG[identifier][_STATE["language"]]
    return template.format(*arguments)


def document_text(identifier: str, *arguments: object) -> str:
    """Текст документа (русский) с подставленными аргументами.
    A document text (Russian) with the arguments substituted."""
    return DOCUMENT_TEXTS[identifier].format(*arguments)
