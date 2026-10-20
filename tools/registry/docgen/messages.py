"""Каталог сообщений сборки документов реестра (русский и английский).
The message catalog of the registry documents build (Russian and English)."""

from __future__ import annotations

# Идентификатор -> {язык: шаблон}; {0}, {1} — места аргументов
# Identifier -> {language: template}; {0}, {1} mark the arguments
CATALOG = {
    "usage": {
        "ru": "Сборка документов для реестра Минцифры: Markdown -> HTML (site/docs) и PDF",
        "en": "Building the documents for the Ministry registry: Markdown -> HTML (site/docs) and PDF",
    },
    "no_browser": {
        "ru": "Не найден Chrome, Edge или Chromium для печати PDF: укажите --browser или соберите только HTML (--html-only)",
        "en": "No Chrome, Edge or Chromium is found to print PDF: pass --browser or build HTML only (--html-only)",
    },
    "print_failed": {
        "ru": "Браузер не напечатал {0} (код {1})",
        "en": "The browser did not print {0} (code {1})",
    },
    "no_sources": {
        "ru": "В {0} нет документов *.md",
        "en": "There are no *.md documents in {0}",
    },
    "written": {
        "ru": "Записан {0}",
        "en": "Written {0}",
    },
}

# Текущий язык / The current language
_STATE = {"language": "ru"}


def set_language(code: str) -> None:
    """Выбор языка сообщений (неизвестный код — русский).
    Chooses the message language (an unknown code means Russian)."""
    if code in ("ru", "en"):
        _STATE["language"] = code
    else:
        _STATE["language"] = "ru"


def text(identifier: str, *arguments: object) -> str:
    """Сообщение на текущем языке с подставленными аргументами.
    A message in the current language with the arguments substituted."""
    return CATALOG[identifier][_STATE["language"]].format(*arguments)
