# Ревью кода: тесты и прогоны наборов W3C, 2026-11-21

Задачи ревью кода по правилу «человек в контуре» (`human_review.md`, раздел 3). Общие пункты О-1…О-8 и порядок работы — в [`../README.md`](../README.md).

## Т-36. Ревью: тесты обёрток: Node.js, WebAssembly, Python, PHP

- **Зачем.** Проверяют ли тесты обёрток то, что обещают: ошибки, освобождение, граничные случаи. Слабый тест обёртки пропускает регрессию в маршалинге.
- **Приоритет:** когда будет время · ~70–120 минут (файлов: 23, строк: ≈ 1822 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-36/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/node/test/document.test.js` (118)
  2. `bindings/node/test/languages.test.js` (48)
  3. `bindings/node/test/move.test.js` (78)
  4. `bindings/node/test/streaming.test.js` (99)
  5. `bindings/node/test/xpath_schema.test.js` (109)
  6. `bindings/wasm/test/streaming.test.mjs` (37)
  7. `bindings/wasm/test/wasm.test.mjs` (93)
  8. `bindings/python/tests/test_document.py` (78)
  9. `bindings/python/tests/test_languages.py` (40)
  10. `bindings/python/tests/test_load.py` (83)
  11. `bindings/python/tests/test_move.py` (68)
  12. `bindings/python/tests/test_streaming.py` (66)
  13. `bindings/python/tests/test_xpath_schema.py` (59)
  14. `bindings/php/tests/DocumentTest.php` (155)
  15. `bindings/php/tests/HeaderSyncTest.php` (65)
  16. `bindings/php/tests/LanguageTest.php` (69)
  17. `bindings/php/tests/MoveTest.php` (95)
  18. `bindings/php/tests/SchemaTest.php` (124)
  19. `bindings/php/tests/Support/AssertionFailed.php` (16)
  20. `bindings/php/tests/Support/TestCase.php` (85)
  21. `bindings/php/tests/Support/TestRunner.php` (88)
  22. `bindings/php/tests/XPathTest.php` (131)
  23. `bindings/php/tests/run.php` (18)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Есть ли тест на каждый путь ошибки (неверный XML, неверное XPath, закрытый документ).
  2. Тесты освобождения: узел после закрытия документа, двойное закрытие.
  3. Нет ли тестов, которые проходят всегда (нет утверждения, `try` глотает ошибку).
  4. Строки не-ASCII (кириллица, эмодзи) в каждом языке.
  5. Одинаковый набор сценариев во всех обёртках — чего не хватает какой-то.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-37. Ревью: тесты обёрток: C# и Java

- **Зачем.** То же для C# (SelfTest) и Java: проверяют ли тесты ошибки, освобождение и потоки.
- **Приоритет:** когда будет время · ~60–90 минут (файлов: 21, строк: ≈ 1421 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-37/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/csharp/Beresta.SelfTest/Beresta.SelfTest.csproj` (32)
  2. `bindings/csharp/Beresta.SelfTest/DocumentChecks.cs` (102)
  3. `bindings/csharp/Beresta.SelfTest/Expect.cs` (28)
  4. `bindings/csharp/Beresta.SelfTest/LanguageChecks.cs` (41)
  5. `bindings/csharp/Beresta.SelfTest/LoadChecks.cs` (68)
  6. `bindings/csharp/Beresta.SelfTest/MoveChecks.cs` (90)
  7. `bindings/csharp/Beresta.SelfTest/Program.cs` (56)
  8. `bindings/csharp/Beresta.SelfTest/StreamingChecks.cs` (92)
  9. `bindings/csharp/Beresta.SelfTest/ValidationWorker.cs` (43)
  10. `bindings/csharp/Beresta.SelfTest/XPathSchemaChecks.cs` (74)
  11. `bindings/java/src/test/java/org/beresta/selftest/Check.java` (56)
  12. `bindings/java/src/test/java/org/beresta/selftest/DocumentChecks.java` (100)
  13. `bindings/java/src/test/java/org/beresta/selftest/Expect.java` (35)
  14. `bindings/java/src/test/java/org/beresta/selftest/FileChecks.java` (38)
  15. `bindings/java/src/test/java/org/beresta/selftest/LanguageChecks.java` (86)
  16. `bindings/java/src/test/java/org/beresta/selftest/LoadChecks.java` (82)
  17. `bindings/java/src/test/java/org/beresta/selftest/MoveChecks.java` (104)
  18. `bindings/java/src/test/java/org/beresta/selftest/SelfTest.java` (64)
  19. `bindings/java/src/test/java/org/beresta/selftest/StreamingChecks.java` (97)
  20. `bindings/java/src/test/java/org/beresta/selftest/ValidationRequest.java` (43)
  21. `bindings/java/src/test/java/org/beresta/selftest/XPathSchemaChecks.java` (90)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `ValidationWorker` / потоковые проверки C#: действительно ли проверяется параллельный доступ.
  2. Java selftest: тесты освобождения и ошибок.
  3. Нет ли тестов без утверждений и с проглоченными исключениями.
  4. Не-ASCII строки.
  5. Сравните набор сценариев с Т-36.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-41. Ревью: модульные тесты: каркас и вспомогательный код

- **Зачем.** Свой тестовый каркас и общие помощники. Ошибка в каркасе (утверждение, которое не останавливает тест, исключение, засчитанное как успех) делает зелёным весь набор.
- **Приоритет:** когда будет время · ~50–80 минут (файлов: 21, строк: ≈ 1188 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-41/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `tests/include/framework/test_framework.hpp` (109)
  2. `tests/include/suites/test_suites.hpp` (114)
  3. `tests/include/support/capi_helpers.hpp` (41)
  4. `tests/include/support/dom_helpers.hpp` (42)
  5. `tests/include/support/parse_helpers.hpp` (35)
  6. `tests/include/support/recording_handler.hpp` (48)
  7. `tests/include/support/resource_helpers.hpp` (39)
  8. `tests/include/support/schema_helpers.hpp` (48)
  9. `tests/include/support/tree_helpers.hpp` (19)
  10. `tests/src/main.cpp` (67)
  11. `tests/src/framework/assertions.cpp` (55)
  12. `tests/src/framework/test_runner.cpp` (58)
  13. `tests/src/support/capi_helpers.cpp` (64)
  14. `tests/src/support/dom_helpers.cpp` (62)
  15. `tests/src/support/parse_helpers.cpp` (59)
  16. `tests/src/support/recording_handler.cpp` (50)
  17. `tests/src/support/recording_handler_access.cpp` (32)
  18. `tests/src/support/recording_handler_events.cpp` (39)
  19. `tests/src/support/resource_helpers.cpp` (70)
  20. `tests/src/support/schema_helpers.cpp` (109)
  21. `tests/src/support/tree_helpers.cpp` (28)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `framework`: проваленное утверждение действительно делает тест красным; исключение в тесте — провал, а не пропуск.
  2. `support`: временные файлы и каталоги удаляются; уникальные имена при параллельном запуске.
  3. `suites` / `main.cpp`: каждый набор зарегистрирован (нет написанных, но не запускаемых тестов).
  4. Код возврата программы тестов при провале не 0.
  5. Помощники не скрывают ошибки (общий `catch (...)`).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-42. Ревью: модульные тесты: Си-интерфейс, DOM, кодировки, обход, вывод

- **Зачем.** Покрывают ли тесты граничные случаи модулей из Т-2, Т-9…Т-14, Т-25. Ищем тесты, которые проверяют меньше, чем обещает имя.
- **Приоритет:** когда будет время · ~70–120 минут (файлов: 20, строк: ≈ 1861 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-42/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `tests/src/capi/capi_bulk_move_tests.cpp` (130)
  2. `tests/src/capi/capi_move_tests.cpp` (124)
  3. `tests/src/capi/capi_parallel_tests.cpp` (145)
  4. `tests/src/capi/capi_stream_tests.cpp` (117)
  5. `tests/src/capi/capi_tests.cpp` (126)
  6. `tests/src/core/copy_tests.cpp` (79)
  7. `tests/src/core/deep_tree_tests.cpp` (94)
  8. `tests/src/core/element_tests.cpp` (74)
  9. `tests/src/core/hierarchy_tests.cpp` (87)
  10. `tests/src/core/namespace_tests.cpp` (97)
  11. `tests/src/core/navigation_tests.cpp` (88)
  12. `tests/src/core/serialize_tests.cpp` (72)
  13. `tests/src/content/content_cursor_tests.cpp` (120)
  14. `tests/src/content/content_plan_tests.cpp` (33)
  15. `tests/src/encoding/encoding_tests.cpp` (67)
  16. `tests/src/encoding/w3c_encoding_tests.cpp` (58)
  17. `tests/src/io/mapped_file_tests.cpp` (108)
  18. `tests/src/i18n/i18n_tests.cpp` (96)
  19. `tests/src/serializer/serializer_tests.cpp` (63)
  20. `tests/src/traversal/traversal_tests.cpp` (83)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Тест без утверждения о результате (только «не упало»).
  2. Нет теста на путь ошибки — какой модуль, какая функция.
  3. Тесты, зависящие от порядка или от локали/часового пояса.
  4. Си-интерфейс: тесты двойного освобождения, NULL-аргументов, исключений через границу.
  5. Список случаев из соответствующих задач ревью, которых нет в тестах.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-43. Ревью: модульные тесты: парсер, DTD, SAX, XInclude, XPath

- **Зачем.** То же для модулей Т-3…Т-8, Т-15…Т-17: бомбы сущностей, глубина, ошибки разбора, неоднозначности XPath.
- **Приоритет:** когда будет время · ~70–120 минут (файлов: 22, строк: ≈ 1783 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-43/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `tests/src/parser/attribute_scale_tests.cpp` (108)
  2. `tests/src/parser/content_tests.cpp` (60)
  3. `tests/src/parser/error_tests.cpp` (70)
  4. `tests/src/parser/flood_tests.cpp` (90)
  5. `tests/src/parser/format_tests.cpp` (75)
  6. `tests/src/parser/fuzz_regression_tests.cpp` (49)
  7. `tests/src/parser/limit_tests.cpp` (64)
  8. `tests/src/parser/w3c_syntax_tests.cpp` (99)
  9. `tests/src/dtd/dtd_entity_tests.cpp` (78)
  10. `tests/src/dtd/dtd_validation_tests.cpp` (70)
  11. `tests/src/dtd/external_resource_tests.cpp` (76)
  12. `tests/src/dtd/w3c_entity_tests.cpp` (112)
  13. `tests/src/dtd/w3c_validity_tests.cpp` (100)
  14. `tests/src/sax/sax_dtd_tests.cpp` (83)
  15. `tests/src/sax/sax_tests.cpp` (66)
  16. `tests/src/xinclude/xinclude_tests.cpp` (76)
  17. `tests/src/xpath/xpath_decimal_tests.cpp` (118)
  18. `tests/src/xpath/xpath_depth_tests.cpp` (96)
  19. `tests/src/xpath/xpath_function_tests.cpp` (80)
  20. `tests/src/xpath/xpath_name_tests.cpp` (37)
  21. `tests/src/xpath/xpath_namespace_tests.cpp` (96)
  22. `tests/src/xpath/xpath_path_tests.cpp` (80)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Есть ли тесты на «бомбы» (billion laughs, quadratic blowup) и предел глубины.
  2. Тесты позиций ошибок (строка/столбец).
  3. XPath: неоднозначности лексера (`*`, `div` как имя), порядок документа для атрибутов.
  4. Тест без утверждения о результате.
  5. Чего не хватает из списков Т-3…Т-8 и Т-15…Т-17.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-44. Ревью: модульные тесты XSD: типы, регулярки, утверждения

- **Зачем.** Тесты значений, регулярных выражений и `xs:assert`: проверяют ли граничные значения и линейность.
- **Приоритет:** когда будет время · ~60–100 минут (файлов: 13, строк: ≈ 1529 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-44/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `tests/src/xsd/xsd_assert_parallel_tests.cpp` (108)
  2. `tests/src/xsd/xsd_assert_rules_tests.cpp` (146)
  3. `tests/src/xsd/xsd_assert_schema_tests.cpp` (171)
  4. `tests/src/xsd/xsd_assert_tests.cpp` (203)
  5. `tests/src/xsd/xsd_assert_value_tests.cpp` (103)
  6. `tests/src/xsd/xsd_assert_xpath_tests.cpp` (121)
  7. `tests/src/xsd/xsd_datatype_tests.cpp` (70)
  8. `tests/src/xsd/xsd_duration_order_tests.cpp` (81)
  9. `tests/src/xsd/xsd_regex_syntax_tests.cpp` (82)
  10. `tests/src/xsd/xsd_regex_table_tests.cpp` (106)
  11. `tests/src/xsd/xsd_regex_unicode_tests.cpp` (116)
  12. `tests/src/xsd/xsd_value_constraint_tests.cpp` (80)
  13. `tests/src/xsd/xsd_schema_lexical_tests.cpp` (142)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Регулярки: есть ли тест линейного времени на «злых» шаблонах с пределом времени.
  2. Даты/длительности: граничные значения из Т-19.
  3. `xs:assert`: `$value`, параллельная проверка (`assert_parallel`) — действительно ли параллельно.
  4. Тест без утверждения о результате.
  5. Чего не хватает из списков Т-17…Т-19, Т-23.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-45. Ревью: модульные тесты XSD: структура схемы, наследование, identity, поток

- **Зачем.** Тесты сборки схемы, redefine, подстановок, identity и потоковой проверки.
- **Приоритет:** когда будет время · ~110–180 минут (файлов: 21, строк: ≈ 2652 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-45/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `tests/src/xsd/xsd_advanced_tests.cpp` (118)
  2. `tests/src/xsd/xsd_attribute_wildcard_tests.cpp` (166)
  3. `tests/src/xsd/xsd_deep_schema_tests.cpp` (120)
  4. `tests/src/xsd/xsd_deep_tests.cpp` (138)
  5. `tests/src/xsd/xsd_derivation_tests.cpp` (133)
  6. `tests/src/xsd/xsd_final_tests.cpp` (68)
  7. `tests/src/xsd/xsd_identity_field_tests.cpp` (201)
  8. `tests/src/xsd/xsd_identity_scope_tests.cpp` (53)
  9. `tests/src/xsd/xsd_identity_tests.cpp` (96)
  10. `tests/src/xsd/xsd_leaf_declaration_tests.cpp` (76)
  11. `tests/src/xsd/xsd_path_tests.cpp` (120)
  12. `tests/src/xsd/xsd_recursive_derivation_tests.cpp` (160)
  13. `tests/src/xsd/xsd_redefine_rules_tests.cpp` (134)
  14. `tests/src/xsd/xsd_redefine_tests.cpp` (131)
  15. `tests/src/xsd/xsd_schema_placement_tests.cpp` (151)
  16. `tests/src/xsd/xsd_schema_rules_tests.cpp` (140)
  17. `tests/src/xsd/xsd_schema_tests.cpp` (90)
  18. `tests/src/xsd/xsd_stream_identity_tests.cpp` (127)
  19. `tests/src/xsd/xsd_stream_tests.cpp` (167)
  20. `tests/src/xsd/xsd_substitution_tests.cpp` (142)
  21. `tests/src/xsd/xsd_xsi_type_root_tests.cpp` (121)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Циклы (наследование, подстановки, redefine) — есть ли тест на каждый вид.
  2. Identity: одинаковые ответы потоковой проверки и проверки дерева (есть ли тест, сравнивающий обе).
  3. Поток: продолжение проверки после ошибки, сброс состояния между документами.
  4. Тест без утверждения о результате.
  5. Чего не хватает из списков Т-20…Т-24.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-46. Ревью: прогон W3C XML Conformance Test Suite

- **Зачем.** Программа прогона решает, что считать «прошло». Ошибка в её логике (неверное сравнение канонического вывода, пропуск случая) даёт ложную зелёную статистику.
- **Приоритет:** когда будет время · ~70–110 минут (файлов: 22, строк: ≈ 1649 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-46/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `tests/conformance/include/conformance/canonical_writer.hpp` (59)
  2. `tests/conformance/include/conformance/case_runner.hpp` (78)
  3. `tests/conformance/include/conformance/catalog.hpp` (76)
  4. `tests/conformance/include/conformance/conformance_error.hpp` (34)
  5. `tests/conformance/include/conformance/console.hpp` (11)
  6. `tests/conformance/include/conformance/report.hpp` (66)
  7. `tests/conformance/include/conformance/test_case.hpp` (88)
  8. `tests/conformance/src/canonical/canonical_escape.cpp` (91)
  9. `tests/conformance/src/canonical/canonical_writer.cpp` (94)
  10. `tests/conformance/src/catalog/catalog_fields.cpp` (97)
  11. `tests/conformance/src/catalog/catalog_loader.cpp` (117)
  12. `tests/conformance/src/common/conformance_error.cpp` (18)
  13. `tests/conformance/src/common/console.cpp` (17)
  14. `tests/conformance/src/main.cpp` (126)
  15. `tests/conformance/src/report/report_markdown.cpp` (84)
  16. `tests/conformance/src/report/report_print.cpp` (42)
  17. `tests/conformance/src/report/report_stats.cpp` (66)
  18. `tests/conformance/src/runner/case_checks.cpp` (98)
  19. `tests/conformance/src/runner/case_judge.cpp` (74)
  20. `tests/conformance/src/runner/case_runner.cpp` (127)
  21. `tests/conformance/CMakeLists.txt` (59)
  22. `tests/conformance/Makefile` (127)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `catalog`: все случаи каталога читаются, ни один не теряется молча (сравните число случаев с официальным).
  2. `canonical`: канонический вывод по правилам набора (порядок атрибутов, экранирование, переводы строк).
  3. Критерии «применимый случай» и «пропущенный» — что исключается и почему, нет ли исключений «для зелёного».
  4. `report`: итоговые числа совпадают с подробным списком.
  5. Код возврата при провале не 0 (CI на него опирается).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-47. Ревью: фаззинг

- **Зачем.** Цели фаззинга и подготовка корпуса. Риски — цель, которая глотает исключения и не ловит падения, словарь и корпус, не доходящие до DTD/XSD.
- **Приоритет:** когда будет время · ~40–70 минут (файлов: 10, строк: ≈ 994 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-47/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `tests/fuzz/include/fuzz/fuzz_input.hpp` (73)
  2. `tests/fuzz/src/common/fuzz_input.cpp` (58)
  3. `tests/fuzz/src/targets/fuzz_parser.cpp` (112)
  4. `tests/fuzz/src/targets/fuzz_schema.cpp` (133)
  5. `tests/fuzz/src/targets/fuzz_xpath.cpp` (111)
  6. `tests/fuzz/tools/cpp_literals.py` (63)
  7. `tests/fuzz/tools/make_corpus.py` (104)
  8. `tests/fuzz/dict/xml.dict` (135)
  9. `tests/fuzz/CMakeLists.txt` (57)
  10. `tests/fuzz/Makefile` (148)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `targets`: каждая цель доходит до интересного кода (DTD, сущности, XSD, XPath), не отбрасывает вход слишком рано.
  2. Исключения библиотеки — ожидаемы, но падение/ASan — нет; не скрывает ли `catch (...)` настоящую ошибку.
  3. Пределы входа (размер, время) разумны.
  4. `tools/make_corpus.py` / `cpp_literals.py`: корпус из тестовых документов полный.
  5. Сборка (CMake/Makefile): флаги `-fsanitize=fuzzer,address,undefined`.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-48. Ревью: прогон W3C XSTS (набор тестов XSD)

- **Зачем.** То же, что Т-46, для XSD: каталог, ожидаемые ответы, известные ограничения. Риски — случай, засчитанный как «известное ограничение», хотя это ошибка.
- **Приоритет:** когда будет время · ~90–150 минут (файлов: 26, строк: ≈ 2265 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-48/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `tests/xsts/include/xsts/case_runner.hpp` (124)
  2. `tests/xsts/include/xsts/catalog.hpp` (83)
  3. `tests/xsts/include/xsts/console.hpp` (11)
  4. `tests/xsts/include/xsts/limitations.hpp` (56)
  5. `tests/xsts/include/xsts/report.hpp` (82)
  6. `tests/xsts/include/xsts/test_case.hpp` (115)
  7. `tests/xsts/include/xsts/xsts_error.hpp` (34)
  8. `tests/xsts/src/catalog/catalog_case.cpp` (94)
  9. `tests/xsts/src/catalog/catalog_fields.cpp` (140)
  10. `tests/xsts/src/catalog/catalog_loader.cpp` (126)
  11. `tests/xsts/src/common/console.cpp` (17)
  12. `tests/xsts/src/common/xsts_error.cpp` (18)
  13. `tests/xsts/src/limitations/limitations_loader.cpp` (98)
  14. `tests/xsts/src/limitations/skip_rules.cpp` (26)
  15. `tests/xsts/src/main.cpp` (146)
  16. `tests/xsts/src/report/report_lists.cpp` (119)
  17. `tests/xsts/src/report/report_markdown.cpp` (77)
  18. `tests/xsts/src/report/report_print.cpp` (71)
  19. `tests/xsts/src/report/report_stats.cpp` (83)
  20. `tests/xsts/src/runner/case_judge.cpp` (61)
  21. `tests/xsts/src/runner/group_runner.cpp` (92)
  22. `tests/xsts/src/runner/instance_validation.cpp` (123)
  23. `tests/xsts/src/runner/schema_hints.cpp` (156)
  24. `tests/xsts/src/runner/schema_loading.cpp` (125)
  25. `tests/xsts/CMakeLists.txt` (60)
  26. `tests/xsts/Makefile` (128)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `catalog`: версии XSD 1.0/1.1, выбор ожидаемого результата по версии.
  2. `limitations`: каждое исключение обосновано, нет широких масок.
  3. `runner`: тайм-аут на случай, падение одного случая не роняет прогон.
  4. `report`: числа совпадают с подробным списком.
  5. Код возврата при провале.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-49. Ревью: отладочный прогон непрошедших случаев XSTS

- **Зачем.** Инструмент отладки по одному случаю (`make run ARGS="--case N"`) и пересборки списка непрошедших. Риски — список, не совпадающий с основным прогоном (другие критерии).
- **Приоритет:** когда будет время · ~50–90 минут (файлов: 18, строк: ≈ 1282 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-49/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `tests/xsts_failing/include/failing/case_executor.hpp` (77)
  2. `tests/xsts_failing/include/failing/debug_run.hpp` (59)
  3. `tests/xsts_failing/include/failing/failing_case.hpp` (53)
  4. `tests/xsts_failing/include/failing/failing_list.hpp` (51)
  5. `tests/xsts_failing/include/failing/list_error.hpp` (30)
  6. `tests/xsts_failing/include/failing/options.hpp` (60)
  7. `tests/xsts_failing/include/failing/regenerate.hpp` (43)
  8. `tests/xsts_failing/src/cli/options.cpp` (86)
  9. `tests/xsts_failing/src/common/list_error.cpp` (9)
  10. `tests/xsts_failing/src/list/breakdown.cpp` (69)
  11. `tests/xsts_failing/src/list/failing_list.cpp` (125)
  12. `tests/xsts_failing/src/list/regenerate.cpp` (96)
  13. `tests/xsts_failing/src/main.cpp` (84)
  14. `tests/xsts_failing/src/runner/case_executor.cpp` (93)
  15. `tests/xsts_failing/src/runner/case_index.cpp` (28)
  16. `tests/xsts_failing/src/runner/debug_run.cpp` (81)
  17. `tests/xsts_failing/CMakeLists.txt` (85)
  18. `tests/xsts_failing/Makefile` (153)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Критерии «прошло» совпадают с Т-48 (общий код или копия?).
  2. `list`: пересборка списка (`make regen`) детерминирована, сортировка стабильна.
  3. `cli`: разбор аргументов, неверный номер случая.
  4. Сборка с `-g -O0` для отладки (Makefile/CMake).
  5. Пути к набору XSTS на всех ОС.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 
