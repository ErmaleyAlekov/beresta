# Ревью кода: DOM, утилиты, сообщения, 2026-11-21

Задачи ревью кода по правилу «человек в контуре» (`human_review.md`, раздел 3). Общие пункты О-1…О-8 и порядок работы — в [`../README.md`](../README.md).

## Т-9. Ревью: DOM: узел, символьные узлы и исключения DOM

- **Зачем.** Базовый класс узла, связи родитель/брат, позиции в исходнике, текст/комментарии/CDATA/PI. Ошибки — висячие указатели после правки дерева и неверные проверки содержимого (`--` в комментарии, `]]>` в CDATA, `?>` в PI).
- **Приоритет:** когда будет время · ~50–80 минут (файлов: 24, строк: ≈ 1253 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-9/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/core/node.hpp` (141)
  2. `include/core/node_type.hpp` (28)
  3. `include/core/source_location.hpp` (28)
  4. `include/core/character_node.hpp` (55)
  5. `include/core/text.hpp` (33)
  6. `include/core/comment.hpp` (35)
  7. `include/core/cdata_section.hpp` (35)
  8. `include/core/processing_instruction.hpp` (64)
  9. `src/core/node/node.cpp` (57)
  10. `src/core/node/node_location.cpp` (47)
  11. `src/core/node/node_relations.cpp` (38)
  12. `src/core/node/node_siblings.cpp` (55)
  13. `src/core/node/node_tree.cpp` (44)
  14. `src/core/character/cdata_section.cpp` (48)
  15. `src/core/character/character_node.cpp` (49)
  16. `src/core/character/character_node_compare.cpp` (20)
  17. `src/core/character/comment.cpp` (51)
  18. `src/core/character/processing_instruction.cpp` (59)
  19. `src/core/character/processing_instruction_access.cpp` (49)
  20. `src/core/character/text.cpp` (36)
  21. `include/exception/dom_exception.hpp` (87)
  22. `include/exception/parse_exception.hpp` (72)
  23. `src/exception/dom_exception.cpp` (72)
  24. `src/exception/parse_exception.cpp` (50)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `node_relations` / `node_siblings`: инварианты двусвязного списка детей после каждой операции; узел из другого документа.
  2. Символьные узлы: запись `--` или завершающего `-` в комментарий, `]]>` в CDATA, `?>` в PI — запрещается ли при правке через API или при сериализации.
  3. `character_node_compare`: сравнение строк с разной нормализацией, пустые строки.
  4. `source_location`: что хранится у узлов, созданных через API (а не разбором).
  5. Исключения DOM: коды и тексты из каталога сообщений, а не литералы.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** ошибок не найдено.
- **Разбор (2026-11-21).** Принято, исправлений нет.


## Т-10. Ревью: DOM: контейнер и элемент

- **Зачем.** Дети, атрибуты, пространства имён, поиск, копирование и перенос узлов. Самое опасное — операции, меняющие дерево во время обхода, и перенос узла внутрь собственного потомка.
- **Приоритет:** желательно до следующего выпуска · ~80–130 минут (файлов: 23, строк: ≈ 2006 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-10/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/core/container_node.hpp` (228)
  2. `include/core/element.hpp` (216)
  3. `include/core/attribute.hpp` (24)
  4. `src/core/container/container_node.cpp` (86)
  5. `src/core/container/container_node_access.cpp` (66)
  6. `src/core/container/container_node_children.cpp` (104)
  7. `src/core/container/container_node_copy.cpp` (96)
  8. `src/core/container/container_node_edit.cpp` (107)
  9. `src/core/container/container_node_internal.cpp` (47)
  10. `src/core/container/container_node_navigation.cpp` (52)
  11. `src/core/container/container_node_search.cpp` (84)
  12. `src/core/container/container_node_search_mutable.cpp` (43)
  13. `src/core/container/container_node_walk.cpp` (123)
  14. `src/core/element/element.cpp` (66)
  15. `src/core/element/element_attributes.cpp` (67)
  16. `src/core/element/element_attributes_bulk.cpp` (100)
  17. `src/core/element/element_content.cpp` (53)
  18. `src/core/element/element_hooks.cpp` (62)
  19. `src/core/element/element_name.cpp` (69)
  20. `src/core/element/element_namespace.cpp` (84)
  21. `src/core/element/element_namespace_attributes.cpp` (87)
  22. `src/core/element/element_namespace_query.cpp` (72)
  23. `src/core/element/element_query.cpp` (70)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `container_node_edit` / `_copy`: вставка предка в собственного потомка (цикл в дереве), вставка узла, уже стоящего в другом месте, вставка документа или атрибута как ребёнка.
  2. `container_node_search_mutable` / `_walk`: изменение дерева во время обхода — итераторы и указатели после удаления текущего узла.
  3. Копирование глубокого дерева: рекурсия без предела (100 000 уровней).
  4. `element_attributes*`: дубликат атрибута при `setAttribute`, атрибут `xmlns:*` как обычный, массовая установка (`_bulk`) с повторами.
  5. `element_namespace*`: поиск URI по префиксу вверх по дереву, переопределение префикса, `xml`-префикс, элемент без объявленного префикса после переноса в другой документ.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** `src/core/element/element_namespace_attributes.cpp:69` — `el.setAttributeNS(XMLNS_NAMESPACE_URI, "xmlns:p", "")`: `lookupNamespaceURI("xmlns")` совпадает с `XMLNS_NAMESPACE_URI`, и `setAttribute("xmlns:p", "")` создаёт объявление пространства имён с пустым URI в обход проверки `PREFIX_EMPTY_URI` из `declareNamespace` (`element_namespace_attributes.cpp:21`).
- **Разбор (2026-11-21).** Подтвердилось, исправлено: `xmlns`, `xmlns:*` и любое имя в пространстве XMLNS `setAttributeNS` передаёт в новый `Element::setNamespaceDeclarationNS` (`element_namespace_query.cpp`). Он требует, чтобы в пространстве XMLNS были только `xmlns` и `xmlns:*` (иначе `XMLNS_ATTRIBUTE_NAMESPACE`), и вызывает `declareNamespace` с её запретами: префиксы `xml` и `xmlns`, пустой URI у префикса. Заодно закрыт вызов `setAttributeNS(XMLNS, "a:b", ...)`, который связывал префикс `a` со служебным пространством.

  **Что нашёл человек и чего не поймала автоматика:** тесты пространств имён проверяли `declareNamespace` и обычные атрибуты, а путь «объявление через setAttributeNS» — нет. **В автопроверки добавлено:** `tests/src/review/review_dom_tests.cpp` — пустой URI, чужое пространство, `a:b` в XMLNS, `xmlns:xml`, правильные `xmlns:p` и `xmlns`.


## Т-11. Ревью: DOM: документ, обход и сериализация

- **Зачем.** Документ (пролог, DOCTYPE, корень, поиск по ID), обход TreeWalker/NodeIterator и вывод XML. Сериализатор — граница наружу: неверное экранирование превращает данные пользователя в разметку.
- **Приоритет:** желательно до следующего выпуска · ~70–120 минут (файлов: 25, строк: ≈ 1745 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-11/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/core/document.hpp` (100)
  2. `include/core/document_type.hpp` (75)
  3. `include/core/xml_declaration.hpp` (47)
  4. `src/core/document/document.cpp` (68)
  5. `src/core/document/document_hooks.cpp` (47)
  6. `src/core/document/document_lookup.cpp` (51)
  7. `src/core/document/document_prolog.cpp` (42)
  8. `src/core/document/document_root.cpp` (70)
  9. `src/core/document/document_type.cpp` (86)
  10. `src/core/document/document_type_access.cpp` (46)
  11. `src/core/document/document_type_model.cpp` (20)
  12. `src/core/document/xml_declaration.cpp` (83)
  13. `include/traversal/node_filter.hpp` (58)
  14. `include/traversal/tree_walker.hpp` (113)
  15. `src/traversal/node_filter.cpp` (34)
  16. `src/traversal/node_iterator.cpp` (72)
  17. `src/traversal/traversal_helpers.cpp` (65)
  18. `src/traversal/tree_walker.cpp` (43)
  19. `src/traversal/tree_walker_moves.cpp` (50)
  20. `src/traversal/tree_walker_order.cpp` (106)
  21. `src/traversal/tree_walker_traverse.cpp` (106)
  22. `include/serializer/xml_serializer.hpp` (112)
  23. `src/serializer/xml_serializer.cpp` (43)
  24. `src/serializer/xml_serializer_output.cpp` (87)
  25. `src/serializer/xml_serializer_pretty.cpp` (121)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `xml_serializer*`: экранирование `<`, `&`, `>` после `]]`, кавычек в атрибутах, `\r`, `\t`, `\n` в атрибутах (иначе потеряются при повторном разборе); символы, недопустимые в выбранной кодировке/версии XML.
  2. Сериализация CDATA, содержащей `]]>`; комментария с `--`; PI с `?>` — что выводится.
  3. Pretty-печать: не добавляет пробелы в смешанное содержимое и в `xml:space="preserve"`.
  4. Обход: NodeIterator/TreeWalker после удаления текущего узла; фильтр, бросающий исключение.
  5. `document_lookup`: поиск по ID после правки дерева (устаревший индекс).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:**
  1. `src/serializer/xml_serializer_pretty.cpp:110` — в `openPrettyElement` проверяется только наличие текстовых детей, а `xml:space="preserve"` на элементе или предках не учитывается: `<pre xml:space="preserve"><span>a</span><span>b</span></pre>` с `prettyPrint = true` получает переносы и отступы между `<span>`, которые при повторном разборе становятся значимыми текстовыми узлами.
  2. `src/core/document/document_type.cpp:25` — конструктор `DocumentType` не проверяет, что `internalSubset` не выходит за `]`, и не проверяет символы `publicId`: `internalSubset = "]><injected/>"` даёт при `DocumentType::serialize` испорченный или подменённый XML.
- **Разбор (2026-11-21).** Оба пункта подтвердились, исправлены.
  1. `preservesSpace(element, lookUp)` (`src/serializer/xml_serializer_space.cpp`): элемент под `xml:space="preserve"` выводится дословно. Предки проверяются только у первого выводимого элемента — у остальных родитель уже выведен с отступами, значит, preserve на нём нет; поэтому проверка стоит O(1) на элемент.
  2. `publicId` — только PubidChar (`isValidPublicId`, как в разборе DTD); подмножество проверяет `utils::isClosedInternalSubset` (`src/utils/xml_internal_subset.cpp`): `]`, кавычка вне объявления, незакрытые литерал, комментарий, инструкция или объявление дают ошибку `DOCTYPE_SUBSET_UNCLOSED`. Это не разбор DTD, а гарантия, что вывод `[...]>` закрывается там, где должен; `]` внутри литералов, комментариев и инструкций допустим.

  **Что нашёл человек и чего не поймала автоматика:** тесты красивого вывода не используют `xml:space`, а `DocumentType` в тестах строится только парсером — из заведомо правильного текста. **В автопроверки добавлено:** `tests/src/review/review_dom_tests.cpp` — свой и унаследованный preserve, сериализация отдельного узла; подмножества с `]` в литерале, комментарии и инструкции (и круговой разбор); выход за `]`, незакрытые литерал и комментарий, `publicId` с `<`.


## Т-12. Ревью: утилиты: строки, UTF-8, имена XML, хеш с ключом

- **Зачем.** Мелкие функции, на которых стоит всё: проверки символов и имён XML, UTF-8, экранирование, таблица имён, хеш с секретным ключом против hash flooding. Ошибка в одной функции проявляется везде.
- **Приоритет:** желательно до следующего выпуска · ~50–80 минут (файлов: 20, строк: ≈ 1241 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-12/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/utils/qualified_name.hpp` (59)
  2. `include/utils/seeded_hash.hpp` (59)
  3. `include/utils/string_utils.hpp` (48)
  4. `include/utils/utf8.hpp` (44)
  5. `include/utils/xml_chars.hpp` (57)
  6. `include/utils/xml_escape.hpp` (49)
  7. `include/utils/xml_name_table.hpp` (27)
  8. `include/utils/xml_tokens.hpp` (35)
  9. `src/utils/namespace_declaration.cpp` (41)
  10. `src/utils/qualified_name.cpp` (46)
  11. `src/utils/seeded_hash.cpp` (124)
  12. `src/utils/string_utils.cpp` (80)
  13. `src/utils/utf8.cpp` (59)
  14. `src/utils/utf8_count.cpp` (30)
  15. `src/utils/utf8_decode.cpp` (103)
  16. `src/utils/xml_chars.cpp` (61)
  17. `src/utils/xml_escape.cpp` (80)
  18. `src/utils/xml_name_chars.cpp` (126)
  19. `src/utils/xml_quote.cpp` (33)
  20. `src/utils/xml_tokens.cpp` (80)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `utf8_decode` / `utf8_count`: границы буфера, overlong-формы, суррогаты, > U+10FFFF — единые правила с декодером из Т-2?
  2. `xml_chars` / `xml_name_chars`: диапазоны XML 1.0 5-го издания и XML 1.1 сверены со спецификацией (сравните с таблицами в стандарте).
  3. `seeded_hash`: источник ключа (случаен ли на каждом запуске, на всех ОС), не утекает ли ключ наружу; качество смешивания для коротких строк.
  4. `xml_name_table`: рост таблицы, коллизии, хранение `string_view` на строки, которые могут умереть.
  5. `xml_escape` / `xml_quote`: тот же набор, что в Т-11, — нет ли двух разных реализаций экранирования.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** ошибок не найдено.
- **Разбор (2026-11-21).** Принято, исправлений нет.


## Т-13. Ревью: десятичная арифметика

- **Зачем.** Своя десятичная арифметика для `xs:decimal` и XPath: деление, округление, нормализация, сравнение текстом. Ошибки — неверный результат на граничных значениях и квадратичное время на очень длинных числах.
- **Приоритет:** когда будет время · ~30–50 минут (файлов: 11, строк: ≈ 795 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-13/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/utils/decimal.hpp` (113)
  2. `include/utils/internal/decimal_digits.hpp` (57)
  3. `src/utils/decimal/decimal_arithmetic.cpp` (43)
  4. `src/utils/decimal/decimal_digits.cpp` (120)
  5. `src/utils/decimal/decimal_division.cpp` (55)
  6. `src/utils/decimal/decimal_format.cpp` (53)
  7. `src/utils/decimal/decimal_long_division.cpp` (60)
  8. `src/utils/decimal/decimal_normalize.cpp` (84)
  9. `src/utils/decimal/decimal_rounding.cpp` (48)
  10. `src/utils/decimal/decimal_text.cpp` (79)
  11. `src/utils/decimal/decimal_text_compare.cpp` (83)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Деление: на ноль, бесконечная дробь (предел знаков), длинное деление на 1000-значных числах — время.
  2. Округление: half-even/half-up в каждом месте использования совпадает со спецификацией XPath/XSD; отрицательные числа, `-0`.
  3. Разбор текста: `+.5`, `5.`, `.`, ведущие нули, пробелы, экспонента (запрещена для decimal).
  4. `decimal_text_compare`: сравнение без перевода в число — `0.10` и `0.1`, `-0` и `0`, разная длина целой части.
  5. Переполнения размеров (`size_t`) при сложении длин.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:**
  1. `src/utils/decimal/decimal_format.cpp:44` — при числе цифр и scale больше предела `long double` (около 4932, а на MSVC, где `long double` — это `double`, уже около 308) `result` и `divisor` оба переполняются в INF, и `result /= divisor` даёт NaN вместо конечного числа: `decimalToDouble` на `"0." + 5000 цифр '1'`, разобранном `readDecimal`.
  2. `src/utils/decimal/decimal_long_division.cpp:43` — на каждой итерации `remainder = std::string(stripLeadingZeros(remainder))`, а `nextQuotientDigit` до 9 раз вызывает `subtractDigits` с двумя выделениями памяти: на делении 1000-значных чисел — около 20 000 лишних выделений и копирований.
- **Разбор (2026-11-21).**
  1. Подтвердилось, исправлено: новый `utils::digitsToLongDouble(digits, exponent)` берёт первые 36 значащих цифр (лишние цифры целой части только увеличивают порядок) и применяет порядок шагами по 10^300, так что промежуточные значения не переполняются. **Найдено при разборе:** та же ошибка была в разборе чисел XPath (`xpath_numbers.cpp`: `number('0.111…')` давало NaN на MSVC); теперь он пользуется той же функцией и заодно точнее на обычных дробях (одно деление вместо накопленного `scale /= 10`).
  2. Верно, но не исправлялось: это стоимость, а не ошибка — точность деления в XPath ограничена `DECIMAL_DIVISION_SCALE = 18`, длина операндов — длиной записи в документе, а 20 000 коротких выделений занимают доли миллисекунды. Оставлено до профилирования.

  **Что нашёл человек и чего не поймала автоматика:** тесты decimal проверяют короткие записи, а на Linux и macOS (80-битный `long double`) порог — 4932 цифры, поэтому CI ошибку не видел. **В автопроверки добавлено:** `tests/src/review/review_dom_tests.cpp` — `"0." + 400 единиц` ≈ 1/9 (порог MSVC), 404-значное целое — бесконечность, то же через `number()` XPath.


## Т-14. Ревью: исключения и каталог сообщений (i18n)

- **Зачем.** Тексты для пользователя на двух языках и подстановка параметров. Ошибки — несовпадение каталогов (ключ есть в одном языке, нет в другом), неверная подстановка `{0}` и смена языка из нескольких потоков.
- **Приоритет:** когда будет время · ~70–110 минут (файлов: 7, строк: ≈ 1631 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-14/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/i18n/i18n.hpp` (125)
  2. `include/i18n/message.hpp` (427)
  3. `src/i18n/catalogs/messages_en.cpp` (417)
  4. `src/i18n/catalogs/messages_ru.cpp` (416)
  5. `src/i18n/i18n_format.cpp` (77)
  6. `src/i18n/i18n_language.cpp` (64)
  7. `src/i18n/i18n_registry.cpp` (105)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `messages_ru.cpp` / `messages_en.cpp`: одинаковый набор и порядок ключей, одинаковые плейсхолдеры в паре строк (проверьте выборочно 20–30 строк).
  2. `i18n_format`: плейсхолдер без аргумента, лишний аргумент, `{` в самом тексте, номер > 9.
  3. `i18n_language`: смена языка во время работы другого потока — гонка?
  4. `i18n_registry`: поиск ключа за пределами таблицы, ключ по умолчанию.
  5. Есть ли в коде библиотеки тексты для пользователя литералами, мимо каталога (поиск `"` в `throw`).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** ошибок не найдено.
- **Разбор (2026-11-21).** Принято, исправлений нет.
