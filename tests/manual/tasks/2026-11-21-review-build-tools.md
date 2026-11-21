# Ревью кода: сборка, CI и инструменты, 2026-11-21

Задачи ревью кода по правилу «человек в контуре» (`human_review.md`, раздел 3). Общие пункты О-1…О-8 и порядок работы — в [`../README.md`](../README.md).

## Т-38. Ревью: сборка и CI: CMake, Makefile, workflow, скрипты выпуска

- **Зачем.** Сборочные файлы и CI определяют, что попадает в релиз и с какими флагами. Риски — секреты и токены в логах, неподписанные или неверные действия, флаги безопасности не на всех платформах, расхождение CMake и Makefile.
- **Приоритет:** желательно до следующего выпуска · ~110–180 минут (файлов: 17, строк: ≈ 2641 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-38/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `CMakeLists.txt` (305)
  2. `Makefile` (667)
  3. `cmake/BerestaBindings.cmake` (185)
  4. `cmake/BerestaJavaSelfTest.cmake` (43)
  5. `cmake/BerestaPlatform.cmake` (140)
  6. `cmake/BerestaWasm.cmake` (58)
  7. `.github/workflows/ci.yml` (193)
  8. `.github/workflows/release.yml` (324)
  9. `.gitverse/workflows/ci.yml` (50)
  10. `.gitverse/workflows/pages.yml` (54)
  11. `.gitverse/workflows/release.yml` (177)
  12. `tools/release/build_linux.sh` (71)
  13. `tools/release/build_windows.sh` (62)
  14. `tools/release/github_assets.sh` (99)
  15. `tools/release/gitverse_release.sh` (114)
  16. `tools/release/lib/build_common.sh` (53)
  17. `tools/release/release_notes.sh` (46)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `.github/workflows/*`: права `permissions` минимальны для каждой задачи; действия (`uses:`) — доверенные, версии зафиксированы; нет `pull_request_target` с кодом из форка.
  2. Секреты не выводятся в лог (`set -x` рядом с токеном, `echo` переменных), `GV_API_TOKEN` в `.gitverse/workflows` — только где нужен.
  3. `tools/release/*.sh`: `set -euo pipefail`, кавычки вокруг переменных, временные файлы, проверки загрузок (контрольные суммы).
  4. CMake и Makefile: одинаковые флаги предупреждений и защиты (`-fstack-protector`, `_FORTIFY_SOURCE`, `/guard:cf`), одинаковый набор исходников.
  5. Makefile: отслеживание зависимостей `.d`, `clean`/`fclean` на Windows и Unix.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-39. Ревью: инструменты: листинг для регистрации и таблицы Unicode

- **Зачем.** Python-скрипты: листинг исходников в PDF (свой писатель PDF и чтение TrueType) и генератор таблиц Unicode. Риски — неверный разбор TTF, неполный листинг, генератор, молча теряющий диапазоны.
- **Приоритет:** когда будет время · ~50–80 минут (файлов: 14, строк: ≈ 1206 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-39/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `tools/legal/listing/__init__.py` (2)
  2. `tools/legal/listing/errors.py` (20)
  3. `tools/legal/listing/layout.py` (85)
  4. `tools/legal/listing/messages.py` (113)
  5. `tools/legal/listing/pdf_document.py` (109)
  6. `tools/legal/listing/pdf_font.py` (97)
  7. `tools/legal/listing/pdf_metadata.py` (94)
  8. `tools/legal/listing/pdf_pages.py` (82)
  9. `tools/legal/listing/pdf_writer.py` (69)
  10. `tools/legal/listing/sources.py` (109)
  11. `tools/legal/listing/ttf_names.py` (34)
  12. `tools/legal/listing/ttf_reader.py` (122)
  13. `tools/legal/make_listing.py` (140)
  14. `tools/unicode/generate_unicode_tables.py` (130)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `ttf_reader.py` / `ttf_names.py`: разбор таблиц шрифта — границы, смещения.
  2. `pdf_*`: экранирование строк в PDF, кодировка кириллицы, перекрёстные ссылки.
  3. `sources.py`: какие файлы попадают в листинг (не пропускаются ли каталоги).
  4. `generate_unicode_tables.py`: диапазоны `First>`/`Last>` в UnicodeData, версия Unicode, детерминированность вывода.
  5. Ошибки — свои классы исключений, сообщения из каталога (`messages.py`).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-40. Ревью: инструменты: документация для реестров и стенд

- **Зачем.** Генератор справочника API (разбор `beresta_c.h`, свой Markdown → HTML/PDF) и скрипты стенда российских ОС. Риски — неполный разбор заголовка (функция не попала в справочник), HTML-инъекция из Markdown, скрипты стенда с непроверенными путями.
- **Приоритет:** когда будет время · ~50–90 минут (файлов: 18, строк: ≈ 1300 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-40/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `tools/registry/build_docs.py` (103)
  2. `tools/registry/doc.css` (62)
  3. `tools/registry/docgen/__init__.py` (2)
  4. `tools/registry/docgen/browser_pdf.py` (63)
  5. `tools/registry/docgen/capi_parse.py` (106)
  6. `tools/registry/docgen/capi_reference.py` (68)
  7. `tools/registry/docgen/errors.py` (15)
  8. `tools/registry/docgen/html_page.py` (56)
  9. `tools/registry/docgen/md_blocks.py` (76)
  10. `tools/registry/docgen/md_inline.py` (48)
  11. `tools/registry/docgen/md_kinds.py` (99)
  12. `tools/registry/docgen/messages.py` (47)
  13. `tools/stand/lib/build.sh` (77)
  14. `tools/stand/lib/environment.sh` (140)
  15. `tools/stand/lib/messages.sh` (138)
  16. `tools/stand/lib/os_info.sh` (35)
  17. `tools/stand/lib/report.sh` (89)
  18. `tools/stand/run_stand.sh` (76)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `capi_parse.py`: все функции и типы `beresta_c.h` попадают в справочник (сравните число функций).
  2. `md_inline.py` / `html_page.py`: экранирование HTML, ссылки `javascript:`.
  3. `browser_pdf.py`: запуск браузера — пути, аргументы, временные файлы.
  4. `tools/stand/*.sh`: `set -euo pipefail`, кавычки, что делается с правами администратора.
  5. Сообщения — из каталога, ошибки — свои классы.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 
