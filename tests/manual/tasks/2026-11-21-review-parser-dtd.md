# Ревью кода: разбор XML и DTD (недоверенный вход), 2026-11-21

Задачи ревью кода по правилу «человек в контуре» (`human_review.md`, раздел 3). Общие пункты О-1…О-8 и порядок работы — в [`../README.md`](../README.md).

## Т-2. Ревью: кодировки, чтение файлов и загрузка внешних ресурсов

- **Зачем.** Первые байты документа приходят отсюда: определение кодировки по BOM и объявлению, декодирование UTF-8/UTF-16 и однобайтовых таблиц, отображение файла в память, разрешение внешних ресурсов. Тесты покрывают корректные файлы и несколько битых; обрезанные суррогаты, нечётная длина UTF-16 и пути вида `../` проверены слабее.
- **Приоритет:** желательно до следующего выпуска · ~60–100 минут (файлов: 27, строк: ≈ 1532 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-2/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/encoding/charset.hpp` (71)
  2. `include/encoding/charsets.hpp` (96)
  3. `include/encoding/encoding_detector.hpp` (82)
  4. `include/encoding/internal/decoding_steps.hpp` (43)
  5. `include/encoding/single_byte_tables.hpp` (23)
  6. `src/encoding/charset_registry.cpp` (75)
  7. `src/encoding/decoded_document.cpp` (67)
  8. `src/encoding/decoding_steps.cpp` (70)
  9. `src/encoding/encoding_detector.cpp` (74)
  10. `src/encoding/single_byte_charset.cpp` (74)
  11. `src/encoding/single_byte_lookup.cpp` (40)
  12. `src/encoding/tables/ibm866.cpp` (25)
  13. `src/encoding/tables/koi8_r.cpp` (25)
  14. `src/encoding/tables/windows_1251.cpp` (25)
  15. `src/encoding/tables/windows_1252.cpp` (25)
  16. `src/encoding/utf16_charset.cpp` (88)
  17. `src/encoding/utf16_units.cpp` (33)
  18. `src/encoding/utf8_charset.cpp` (55)
  19. `include/io/file_io.hpp` (42)
  20. `include/io/internal/file_mapping.hpp` (37)
  21. `include/io/mapped_file.hpp` (71)
  22. `src/io/file_io.cpp` (57)
  23. `src/io/mapped_file.cpp` (44)
  24. `src/io/platform/file_mapping_posix.cpp` (67)
  25. `src/io/platform/file_mapping_windows.cpp` (64)
  26. `include/parser/resource_resolver.hpp` (86)
  27. `src/parser/resources/file_resource_resolver.cpp` (73)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Декодеры: обрезанная последовательность UTF-8 в конце буфера, суррогаты в UTF-8 (CESU), одиночный суррогат и нечётное число байт в UTF-16, overlong-формы; что попадает в вывод — ошибка или U+FFFD?
  2. Определение кодировки: BOM противоречит `encoding="..."`; объявление длиннее буфера; регистр и пробелы в имени кодировки.
  3. Однобайтовые таблицы (`src/encoding/tables/` — сгенерированы): только индексация — байт 0x80–0xFF всегда в границах таблицы, «дыры» таблицы дают ошибку, а не 0.
  4. `mapped_file` / `file_mapping_*`: пустой файл, файл > 4 ГБ на 32-битной сборке, ошибка открытия — дескриптор/хэндл закрывается на всех путях, кириллица в пути на Windows (широкие функции).
  5. `file_resource_resolver`: выход за базовый каталог (`../`, абсолютный путь, `file://` с хостом, `%2e%2e`), запрет внешних ресурсов по умолчанию соблюдается.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:**
  1. `src/encoding/encoding_detector.cpp:54` — `readDeclaredEncoding` ищет подстроку "encoding" без проверки пробела перед ней и срабатывает на любом псевдоатрибуте с таким окончанием: `<?xml badencoding="cp1251" encoding="utf-8"?><a/>` выбирает неверную кодировку.
  2. `src/parser/resources/file_resource_resolver.cpp:48–53` — граница каталога проверяется только по записи пути (`lexically_normal`, `lexically_relative`) без раскрытия символических ссылок: при ссылке `корень/external -> /outside` запрос `external/secret.xml` проходит проверку и читает файл за пределами корня.
- **Разбор (2026-11-21).** Оба пункта подтвердились, исправлены.
  1. Псевдоатрибут ищется только после пробела (`encoding_detector.cpp`).
  2. Путь проверяется дважды: по записи и по настоящему пути на диске (`weakly_canonical` для цели и корня); ошибка `weakly_canonical` — отказ. Деструктор интерфейса вынесен в `src/parser/resources/resource_resolver.cpp` (лимит функций в файле).

  **Что нашёл человек и чего не поймала автоматика:** обход корня через символическую ссылку — фаззинг и тесты работают с обычными файлами; подмена кодировки чужим псевдоатрибутом — такой декларации нет ни в одном наборе W3C. **В автопроверки добавлено:** `tests/src/review/review_xinclude_tests.cpp` (ссылка внутри корня; на Windows без прав на ссылки — только обычный путь), `tests/src/review/review_parser_tests.cpp` (`badencoding`, `xencoding`).


## Т-3. Ревью: сканер, входной стек, ссылки и бюджет раскрытий

- **Зачем.** Низший уровень разбора: позиция в буфере, стек входов при раскрытии сущностей, декодирование ссылок `&#x...;` и значений атрибутов, пространства имён. Здесь живёт защита от «бомб» сущностей; ошибка в счёте бюджета или в смещениях даёт либо DoS, либо чтение за буфером.
- **Приоритет:** желательно до следующего выпуска · ~50–90 минут (файлов: 21, строк: ≈ 1301 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-3/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/parser/xml_scanner.hpp` (130)
  2. `include/parser/internal/attribute_value_decoder.hpp` (67)
  3. `include/parser/internal/declaration_reader.hpp` (42)
  4. `include/parser/internal/expansion_budget.hpp` (41)
  5. `include/parser/internal/input_stack.hpp` (87)
  6. `include/parser/internal/namespace_context.hpp` (52)
  7. `include/parser/internal/parse_pipeline.hpp` (39)
  8. `include/parser/internal/post_processing.hpp` (39)
  9. `include/parser/internal/reference_decoding.hpp` (26)
  10. `src/parser/scanner/xml_scanner.cpp` (47)
  11. `src/parser/scanner/xml_scanner_error.cpp` (49)
  12. `src/parser/scanner/xml_scanner_position.cpp` (97)
  13. `src/parser/scanner/xml_scanner_read.cpp` (65)
  14. `src/parser/internal/attribute_value_decoder.cpp` (125)
  15. `src/parser/internal/declaration_reader.cpp` (93)
  16. `src/parser/internal/expansion_budget.cpp` (31)
  17. `src/parser/internal/input_stack.cpp` (42)
  18. `src/parser/internal/input_stack_entities.cpp` (59)
  19. `src/parser/internal/input_stack_offsets.cpp` (18)
  20. `src/parser/internal/namespace_context.cpp` (78)
  21. `src/parser/internal/reference_decoding.cpp` (74)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Каждое продвижение позиции сканера: проверка остатка до чтения (`len > size - pos`, а не `pos + len > size`).
  2. `input_stack*`: смещения и строки/столбцы после выхода из сущности; сущность, ссылающаяся на себя (прямо и через цепочку) — где обнаруживается цикл.
  3. `expansion_budget`: учитываются ли и глубина, и суммарный объём, и коэффициент усиления; переполнение счётчика при огромных значениях; сброс бюджета между документами.
  4. `reference_decoding`: `&#0;`, `&#x110000;`, суррогаты, очень длинные цифры (`&#00000…65;`), переполнение при накоплении числа.
  5. `attribute_value_decoder`: нормализация пробелов для CDATA и не-CDATA атрибутов, `<` из раскрытой сущности в значении атрибута.
  6. `namespace_context`: префикс `xml`/`xmlns`, отмена объявления `xmlns=""`, пустой префикс в XML 1.1 (`xmlns:p=""`).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:**
  1. `src/parser/internal/expansion_budget.cpp:15` — `_bytes += bytes` без проверки переполнения `std::size_t`: при лимите около `SIZE_MAX` сумма может обернуться в малое число и обойти `_maxBytes`.
  2. `src/parser/internal/declaration_reader.cpp` (`stripTextDeclaration`) — при неизвестном псевдоатрибуте перед encoding (`<?xml foo="1" encoding="UTF-8"?>`) парсер сообщает об отсутствии encoding.
- **Разбор (2026-11-21).** Оба пункта подтвердились, исправлены.
  1. Объём сравнивается с остатком лимита (`bytes > _maxBytes - _bytes`) до сложения; инвариант `_bytes <= _maxBytes` держится всегда.
  2. Нет encoding и сразу `?>` — «encoding обязателен», иначе — «допустимы только version и encoding (по порядку)».

  **Что нашёл человек и чего не поймала автоматика:** переполнение видно только при лимите, близком к `SIZE_MAX`, — ни один тест так не настраивает парсер; неточное сообщение не меняет исход разбора, поэтому наборы W3C его не замечают. **В автопроверки добавлено:** `tests/src/review/review_parser_tests.cpp` — лимит `SIZE_MAX - 1` и два заряда, сумма которых переполнила бы `size_t`; оба сообщения текстовой декларации.


## Т-4. Ревью: сеанс разбора, построитель DOM и конвейер

- **Зачем.** Грамматика документа поверх сканера: пролог, DOCTYPE, элементы, текст, ссылки, сбор ошибок и построение дерева. Ошибки здесь — пропущенные проверки корректности (well-formedness) или исключение, оставляющее дерево наполовину построенным.
- **Приоритет:** желательно до следующего выпуска · ~80–140 минут (файлов: 27, строк: ≈ 2050 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-4/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/parser/xml_parser.hpp` (72)
  2. `include/parser/parser_options.hpp` (126)
  3. `include/parser/parse_session.hpp` (169)
  4. `include/parser/dom_builder.hpp` (70)
  5. `include/parser/error_handler.hpp` (98)
  6. `src/parser/xml_parser.cpp` (57)
  7. `src/parser/xml_parser_pipeline.cpp` (53)
  8. `src/parser/session/parse_declaration.cpp` (59)
  9. `src/parser/session/parse_doctype.cpp` (114)
  10. `src/parser/session/parse_element.cpp` (116)
  11. `src/parser/session/parse_element_events.cpp` (90)
  12. `src/parser/session/parse_markup.cpp` (104)
  13. `src/parser/session/parse_namespaces.cpp` (128)
  14. `src/parser/session/parse_session.cpp` (121)
  15. `src/parser/session/parse_session_errors.cpp` (47)
  16. `src/parser/session/parse_session_locator.cpp` (54)
  17. `src/parser/session/parse_text.cpp` (87)
  18. `src/parser/session/parse_text_reference.cpp` (111)
  19. `src/parser/builder/dom_builder.cpp` (47)
  20. `src/parser/builder/dom_builder_content.cpp` (75)
  21. `src/parser/builder/dom_builder_prolog.cpp` (51)
  22. `src/parser/errors/collecting_error_handler.cpp` (32)
  23. `src/parser/errors/collecting_error_handler_access.cpp` (30)
  24. `src/parser/errors/error_handler.cpp` (31)
  25. `src/parser/pipeline/parse_pipeline.cpp` (40)
  26. `src/parser/pipeline/post_processing.cpp` (38)
  27. `src/parser/pipeline/sax_pipeline.cpp` (30)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Каждое правило well-formedness из XML 1.0/1.1 (теги, атрибуты-дубликаты, `]]>` в тексте, PI с именем `xml`) — проверяется ли и в каком файле.
  2. Глубина вложенности элементов: есть ли предел; рекурсия в разборе (переполнение стека на 100 000 вложенных `<a>`).
  3. Исключение посреди разбора: что остаётся в `dom_builder` (утечки узлов, висячие указатели на родителя), повторное использование парсера после ошибки.
  4. `collecting_error_handler`: предел числа собранных ошибок, позиции (строка/столбец) после CRLF и после раскрытия сущностей.
  5. `parser_options` → конвейер: каждая опция действительно доходит до места применения; значения по умолчанию безопасны (внешние ресурсы выключены).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:**
  1. `src/parser/builder/dom_builder_content.cpp:67` — каждый кусок текста склеивается в новую строку `previous.getValue() + std::string(text)` вместо дописывания: время O(N²) на тысячах подряд идущих ссылок на сущности или секций `<![CDATA[x]]>` при `coalesceCData`.
  2. `src/parser/session/parse_declaration.cpp:17` — на `<?xml?>` и `<?XML version="1.0"?>` в начале документа конструкция уходит в `parseProcessingInstruction()` и выдаёт ложную ошибку «декларация не на своём месте» вместо сообщения об испорченной декларации.
- **Разбор (2026-11-21).** Оба пункта подтвердились, исправлены.
  1. Новый метод `CharacterNode::appendValue(std::string_view)` (`src/core/character/character_node_append.cpp`): дописывание за O(длина куска), `validateValue()` проверяет кусок вместе с двумя последними байтами прежнего значения — запреты `--` и `]]>` на стыке ловятся так же, как при `setValue`. `DomBuilder::appendText` пользуется им.
  2. Цель `xml` (в любом регистре) на смещении 0 главного входа — новое сообщение `XML_DECLARATION_MALFORMED`; в остальных местах — прежнее `XML_DECLARATION_PLACEMENT`.

  **Что нашёл человек и чего не поймала автоматика:** квадратичное время не ломает ни одного результата, а тесты потока текста состоят из одного большого куска; неточное сообщение не меняет исход разбора. **В автопроверки добавлено:** `tests/src/review/review_parser_tests.cpp` — 100 000 секций CDATA с `coalesceCData` (один узел; при возврате к склейке тест станет заметно медленным), `appendValue` с запретами на стыке, три варианта `<?xml`.


## Т-5. Ревью: SAX и XInclude

- **Зачем.** SAX отдаёт события в код пользователя (его обработчик может бросить исключение или остановить разбор), XInclude подгружает чужие документы. Риски — повторный вход, циклические включения и выход за базовый каталог.
- **Приоритет:** желательно до следующего выпуска · ~30–60 минут (файлов: 15, строк: ≈ 845 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-5/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/sax/content_handler.hpp` (89)
  2. `include/sax/locator.hpp` (51)
  3. `include/sax/sax_parser.hpp` (58)
  4. `include/sax/sax_types.hpp` (44)
  5. `src/sax/content_handler.cpp` (36)
  6. `src/sax/content_handler_elements.cpp` (39)
  7. `src/sax/content_handler_misc.cpp` (34)
  8. `src/sax/locator.cpp` (25)
  9. `src/sax/sax_parser.cpp` (58)
  10. `include/xinclude/internal/xinclude_processor.hpp` (67)
  11. `include/xinclude/xinclude.hpp` (36)
  12. `src/xinclude/xinclude_fallback.cpp` (81)
  13. `src/xinclude/xinclude_loading.cpp` (88)
  14. `src/xinclude/xinclude_pointer.cpp` (71)
  15. `src/xinclude/xinclude_processor.cpp` (68)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. SAX: исключение из обработчика пользователя — состояние парсера после него, нет ли двойного `endElement`; locator действителен только во время события?
  2. SAX: порядок событий для PI и комментариев из DTD, `startPrefixMapping` до `startElement`.
  3. XInclude: цикл `a.xml → b.xml → a.xml`, включение самого себя через `href=""`, предел глубины и общего объёма.
  4. XInclude: `parse="text"` с неверной кодировкой, `xpointer` с непредусмотренной схемой, fallback внутри fallback.
  5. XInclude: базовый URI вложенного документа и запрет внешних ресурсов по умолчанию.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:**
  1. `src/xinclude/xinclude_loading.cpp:52` — пустой `href=""` всегда отклоняется ошибкой `XINCLUDE_NO_HREF`, хотя по XInclude 1.0 `href=""` с `xpointer` означает ссылку на текущий документ; на `<xi:include href=""/>` подстановка уходит в fallback.
  2. `src/xinclude/xinclude_loading.cpp:62` — нет учёта общего объёма развёрнутых данных (проверяется только глубина `_stack.size() >= _options.maxEntityDepth`): цепочка из 10 файлов без циклов, где каждый включает следующий 10 раз (10^10 подстановок), исчерпает память. Корневой `options.baseUri` не кладётся в `_stack`: прямой цикл `a.xml` → `a.xml` делает лишний проход загрузки и разбора до обнаружения повтора.
  3. `src/xinclude/xinclude_fallback.cpp:47` — цикл по детям перезаписывает `fallback` при каждом `<xi:fallback>`: на `<xi:include href="missing.xml"><xi:fallback>1</xi:fallback><xi:fallback>2</xi:fallback></xi:include>` молча подставляется второй блок вместо ошибки (XInclude 1.0 запрещает несколько `xi:fallback`).
- **Разбор (2026-11-21).**
  1. Подтвердилось. Исправлено то, что нарушает правила: `xi:include` без `href` и без `xpointer` теперь фатальная ошибка (`XINCLUDE_NO_LOCATION`), а не переход к fallback (XInclude 1.0, 3.1). Включение части текущего документа (`href=""` + `xpointer`) по-прежнему не поддерживается и честно сообщает об этом: это новая возможность со своим риском (копии поддерева растут экспоненциально, нужен отдельный учёт), она вынесена в отдельную задачу.
  2. Подтвердилось, исправлено: `XIncludeProcessor::charge` — первая загрузка каждого URI бесплатна, повторные ограничены `maxEntityExpansions` (число) и `maxExpandedSize` (байты, сравнение с остатком без переполнения). Цепочка «каждый включает следующий много раз» останавливается на пределе, документы из разных файлов не затронуты. Лишний проход при цикле `a.xml` → `a.xml` оставлен: `options.baseUri` пользователя и URI резолвера записаны по-разному, сравнение ненадёжно, а цикл и так ловится на втором шаге.
  3. Подтвердилось, исправлено: `checkIncludeElement` до загрузки — больше одного `xi:fallback` даёт ошибку (`XINCLUDE_FALLBACK_MULTIPLE`), даже если ресурс доступен.

  **Найдено при разборе:** в `loadXml` URI не снимался со стека, если во включённом файле была ошибка без fallback; после перехода к `xi:fallback` следующее включение того же файла ложно считалось циклом и роняло весь разбор. Теперь URI снимается и при исключении.

  **Что нашёл человек и чего не поймала автоматика:** тесты XInclude проверяли только удачные пути и один цикл; набора W3C для XInclude в проекте нет. **В автопроверки добавлено:** `tests/src/review/review_xinclude_tests.cpp` — два fallback, include без href и xpointer, пять повторов при пределе 3 и 4, повторное включение файла после fallback.


## Т-6. Ревью: DTD: чтение и разбор объявлений

- **Зачем.** Внутреннее и внешнее подмножество DTD — самый «изобретательный» недоверенный вход: параметрические сущности внутри объявлений, условные секции, литералы с кавычками. Ошибки — неверная граница литерала, PE-ссылка, разрывающая объявление, бесконечный цикл.
- **Приоритет:** желательно до следующего выпуска · ~50–90 минут (файлов: 13, строк: ≈ 1356 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-6/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/dtd/dtd_reader.hpp` (175)
  2. `include/dtd/dtd_parser.hpp` (92)
  3. `src/dtd/reader/dtd_reader.cpp` (71)
  4. `src/dtd/reader/dtd_reader_entities.cpp` (121)
  5. `src/dtd/reader/dtd_reader_frames.cpp` (62)
  6. `src/dtd/reader/dtd_reader_literals.cpp` (93)
  7. `src/dtd/reader/dtd_reader_tokens.cpp` (44)
  8. `src/dtd/parser/dtd_parser.cpp` (87)
  9. `src/dtd/parser/dtd_parser_attributes.cpp` (142)
  10. `src/dtd/parser/dtd_parser_elements.cpp` (142)
  11. `src/dtd/parser/dtd_parser_entities.cpp` (127)
  12. `src/dtd/parser/dtd_parser_markup.cpp` (94)
  13. `src/dtd/parser/dtd_parser_misc.cpp` (106)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `dtd_reader_frames`: PE-ссылка, открывающаяся в одном объявлении и закрывающаяся в другом (правило «proper declaration/PE nesting»).
  2. `dtd_reader_literals`: литерал без закрывающей кавычки до конца входа; `%` и `&` внутри литерала значения сущности.
  3. Условные секции `<![INCLUDE[ <![IGNORE[ ... ]]> ]]>`: вложенность, предел глубины, `]]>` внутри IGNORE.
  4. `dtd_parser_elements`: модели `(a|b)*`, `(#PCDATA|a)*`, смешанные без `*`, пустая группа; глубина вложенных групп.
  5. `dtd_parser_attributes`: повторное объявление атрибута (первое побеждает), `ID` с значением по умолчанию, `NOTATION` без объявленной нотации.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** `src/dtd/parser/dtd_parser_misc.cpp:42` — в `parseConditionalSection` нет ограничения глубины рекурсии при вложенных `<![INCLUDE[ ... ]]>` (в отличие от `parseGroup` в `dtd_parser_elements.cpp:89`, где стоит `depth > 64`). Несколько тысяч вложенных `<![INCLUDE[` во внешнем DTD переполнят стек и уронят процесс (пункт О-4).
- **Разбор (2026-11-21).** Подтвердилось, исправлено: счётчик `DtdParser::_sectionDepth`, глубже 64 — ошибка `LIMIT_DEPTH` (как у групп модели). Секции `IGNORE` и так пропускаются без рекурсии.

  **Что нашёл человек и чего не поймала автоматика:** фаззер не строит внешние DTD, а глубина условных секций в наборах W3C — 2–3 уровня. **В автопроверки добавлено:** `tests/src/review/review_dtd_content_tests.cpp` — 10 уровней разбираются, 5000 — ошибка предела, а не падение.


## Т-7. Ревью: DTD: модель, значения по умолчанию и проверка

- **Зачем.** Модель DTD хранит объявления, а валидатор сверяет с ними документ: содержимое элементов, атрибуты, ID/IDREF, standalone. Риски — неверная подстановка значений по умолчанию и неполные проверки validity constraints.
- **Приоритет:** когда будет время · ~50–80 минут (файлов: 16, строк: ≈ 1155 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-7/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/dtd/dtd_model.hpp` (189)
  2. `include/dtd/dtd_validator.hpp` (115)
  3. `include/dtd/dtd_declaration_checks.hpp` (46)
  4. `include/dtd/attribute_defaults.hpp` (38)
  5. `src/dtd/attribute_defaults.cpp` (72)
  6. `src/dtd/model/dtd_model_add.cpp` (63)
  7. `src/dtd/model/dtd_model_find.cpp` (66)
  8. `src/dtd/model/dtd_model_iterate.cpp` (26)
  9. `src/dtd/model/dtd_model_notes.cpp` (34)
  10. `src/dtd/model/dtd_model_query.cpp` (20)
  11. `src/dtd/validator/dtd_attribute_checks.cpp` (118)
  12. `src/dtd/validator/dtd_declaration_checks.cpp` (51)
  13. `src/dtd/validator/dtd_validator.cpp` (82)
  14. `src/dtd/validator/dtd_validator_attributes.cpp` (110)
  15. `src/dtd/validator/dtd_validator_content.cpp` (71)
  16. `src/dtd/validator/dtd_validator_standalone.cpp` (54)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `attribute_defaults`: значения по умолчанию добавляются только объявленным атрибутам, нормализуются по типу; `#FIXED` с другим значением — ошибка.
  2. ID/IDREF/IDREFS: дубликат ID, IDREF на несуществующий ID (проверка в конце документа), ID у двух атрибутов одного элемента.
  3. `dtd_validator_standalone`: `standalone="yes"` и атрибут по умолчанию из внешнего подмножества, пробельные символы в element content.
  4. `dtd_validator_content`: смешанное содержимое, EMPTY с комментарием/PI внутри, ANY.
  5. `dtd_model_*`: поиск по имени при совпадении префиксов, порядок итерации стабилен.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** `src/dtd/validator/dtd_validator_attributes.cpp:74` — в `checkValueByType` после `reportValidity` о неверном числе токенов у одиночного атрибута (ID, IDREF, ENTITY, NMTOKEN, например `id="a b"`) нет `return`, и цикл ниже регистрирует оба токена: на `<root id="a b"><ref target="b"/></root>` (id — ID, target — IDREF) `b` засчитывается существующим идентификатором, и ошибки `IDREF_DANGLING` нет.
- **Разбор (2026-11-21).** Подтвердилось, исправлено: после ошибки значения — `return`, части неверного значения не регистрируются.

  **Что нашёл человек и чего не поймала автоматика:** наборы W3C проверяют, что документ невалиден, но не число ошибок — пропущенная вторая ошибка исход не меняет. **В автопроверки добавлено:** `tests/src/review/review_dtd_content_tests.cpp` — `id="a b"` и ссылка на `b` дают ровно две ошибки.


## Т-8. Ревью: модели содержимого: автоматы и сопоставление

- **Зачем.** Общий механизм для DTD и XSD: частицы, группы `sequence/choice/all`, minOccurs/maxOccurs, подстановки. Ошибка здесь — экспоненциальное время на «неудачной» модели или неверный ответ на граничных occurs.
- **Приоритет:** желательно до следующего выпуска · ~70–110 минут (файлов: 18, строк: ≈ 1680 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-8/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/content/content_cursor.hpp` (184)
  2. `include/content/content_matcher.hpp` (94)
  3. `include/content/internal/automaton_step.hpp` (104)
  4. `include/content/particle.hpp` (152)
  5. `src/content/content_automaton_climb.cpp` (106)
  6. `src/content/content_automaton_descend.cpp` (101)
  7. `src/content/content_automaton_finish.cpp` (69)
  8. `src/content/content_automaton_step.cpp` (50)
  9. `src/content/content_config_set.cpp` (51)
  10. `src/content/content_cursor.cpp` (93)
  11. `src/content/content_cursor_automaton.cpp` (70)
  12. `src/content/content_fast_path.cpp` (124)
  13. `src/content/content_matcher.cpp` (97)
  14. `src/content/content_matcher_all.cpp` (70)
  15. `src/content/content_matcher_groups.cpp` (70)
  16. `src/content/content_plan.cpp` (73)
  17. `src/content/particle.cpp` (106)
  18. `src/content/substitution.cpp` (66)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Сложность: `maxOccurs` = 1 000 000 и вложенные `(a?){n}` — строится ли автомат за линейное время/память, есть ли предел.
  2. `content_automaton_*`: выход из вложенной группы (climb/descend) при `minOccurs=0`, пустая группа, `all` с `maxOccurs=0` у частицы.
  3. `content_fast_path`: совпадают ли ответы быстрого пути с общим автоматом (те же входы — тот же результат).
  4. `substitution`: группа подстановки с циклом A → B → A, абстрактный голова-элемент, `block`.
  5. `content_cursor`: состояние курсора после ошибки — можно ли продолжать проверку следующих детей.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:**
  1. `src/content/content_matcher.cpp:29-54` (`ContentMatcher::matchRepeated`) — при частице, которая может совпасть с пустотой, и большом `minOccurs="1000000000"` множество позиций не меняется, и цикл делает `minOccurs` итераций `matchOnce` даже на пустом списке детей.
  2. `src/content/content_matcher_all.cpp:10-18` (`requiredUsed`) — сдвиг `used >> index` без проверки `index < 64`, хотя соседняя `expandAllState` (`content_matcher_all.cpp:50`) её имеет: на `xs:all` с 65 и больше детьми — неопределённое поведение.
- **Разбор (2026-11-21).** Оба пункта подтвердились, исправлены.
  1. Воспроизведено: `<xs:sequence maxOccurs="unbounded"><xs:sequence minOccurs="1000000000" ...><xs:element name="a" minOccurs="0"/>` на `<r><a/><a/></r>` — больше 2 минут без ответа. Если шаг не изменил множество позиций, оставшиеся до `minOccurs` повторы пропускаются (дальше оно уже не меняется). Теперь 1 мс.
  2. Сдвиг защищён `index < 64`. **Найдено при разборе:** за неопределённым поведением стояла ошибка смысла — маска использованных частиц 64-битная, и в обоих путях (дерево и поток) частицы `xs:all` после 64-й молча считались недопустимыми: правильный документ отвергался. Схема с `xs:all` больше 64 частиц теперь честно отвергается ошибкой `SCHEMA_ALL_TOO_LARGE` (`schema_builder_occurs.cpp`).

  **Что нашёл человек и чего не поймала автоматика:** зависание проявляется только на вложенных счётчиках с пустой частицей, а XSTS таких схем не содержит; UB под санитайзером видно только на `xs:all` больше 64, которых в наборах тоже нет. **В автопроверки добавлено:** `tests/src/review/review_dtd_content_tests.cpp` — огромный `minOccurs` (дерево и поток) и `xs:all` на 64 и 65 частиц.
