# Ревью кода: бенчмарки и примеры, 2026-11-21

Задачи ревью кода по правилу «человек в контуре» (`human_review.md`, раздел 3). Общие пункты О-1…О-8 и порядок работы — в [`../README.md`](../README.md).

## Т-50. Ревью: бенчмарки C++: ядро замеров

- **Зачем.** Честность сравнения с другими библиотеками: прогрев, повторы, измерение памяти, проверка, что движки делают одну и ту же работу. Риски — замер, который оптимизатор выбросил, неравные условия.
- **Приоритет:** когда будет время · ~50–80 минут (файлов: 18, строк: ≈ 1145 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-50/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `benchmarks/include/bench/engine.hpp` (92)
  2. `benchmarks/include/bench/measure.hpp` (60)
  3. `benchmarks/include/bench/options.hpp` (25)
  4. `benchmarks/include/bench/report.hpp` (19)
  5. `benchmarks/include/bench/workload.hpp` (40)
  6. `benchmarks/src/core/engine.cpp` (34)
  7. `benchmarks/src/core/engine_validation.cpp` (20)
  8. `benchmarks/src/core/measure.cpp` (78)
  9. `benchmarks/src/core/memory.cpp` (75)
  10. `benchmarks/src/core/options.cpp` (50)
  11. `benchmarks/src/core/registry.cpp` (54)
  12. `benchmarks/src/core/report.cpp` (70)
  13. `benchmarks/src/core/report_checks.cpp` (36)
  14. `benchmarks/src/core/workload_article.cpp` (57)
  15. `benchmarks/src/core/workload_catalog.cpp` (77)
  16. `benchmarks/src/main.cpp` (107)
  17. `benchmarks/CMakeLists.txt` (89)
  18. `benchmarks/Makefile` (162)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `measure`: прогрев, медиана/минимум, защита от выбрасывания результата оптимизатором.
  2. `memory`: как меряется пик памяти на каждой ОС.
  3. `engine_validation` / `report_checks`: проверка, что результаты движков совпадают (иначе сравнение нечестно).
  4. `workload_*`: одинаковые данные для всех движков, детерминированная генерация.
  5. `options`: разбор аргументов.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-51. Ревью: бенчмарки C++: движки

- **Зачем.** Адаптеры libxml2, pugixml, RapidXML, tinyxml2, Xerces и самой «Берёсты». Риски — один движок делает меньше работы (без проверки, без пространств имён), чем другой.
- **Приоритет:** когда будет время · ~40–60 минут (файлов: 21, строк: ≈ 973 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-51/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `benchmarks/include/bench/engines/beresta_engine.hpp` (38)
  2. `benchmarks/include/bench/engines/libxml2_engine.hpp` (48)
  3. `benchmarks/include/bench/engines/pugixml_engine.hpp` (36)
  4. `benchmarks/include/bench/engines/rapidxml_engine.hpp` (40)
  5. `benchmarks/include/bench/engines/tinyxml2_engine.hpp` (31)
  6. `benchmarks/include/bench/engines/xerces_engine.hpp` (43)
  7. `benchmarks/src/engines/beresta/beresta_engine.cpp` (67)
  8. `benchmarks/src/engines/beresta/beresta_queries.cpp` (63)
  9. `benchmarks/src/engines/libxml2/libxml2_engine.cpp` (44)
  10. `benchmarks/src/engines/libxml2/libxml2_queries.cpp` (54)
  11. `benchmarks/src/engines/libxml2/libxml2_stream.cpp` (45)
  12. `benchmarks/src/engines/libxml2/libxml2_traverse.cpp` (41)
  13. `benchmarks/src/engines/pugixml/pugixml_engine.cpp` (56)
  14. `benchmarks/src/engines/pugixml/pugixml_queries.cpp` (32)
  15. `benchmarks/src/engines/rapidxml/rapidxml_engine.cpp` (65)
  16. `benchmarks/src/engines/rapidxml/rapidxml_output.cpp` (18)
  17. `benchmarks/src/engines/tinyxml2/tinyxml2_engine.cpp` (57)
  18. `benchmarks/src/engines/tinyxml2/tinyxml2_output.cpp` (13)
  19. `benchmarks/src/engines/xerces/xerces_engine.cpp` (54)
  20. `benchmarks/src/engines/xerces/xerces_traverse.cpp` (63)
  21. `benchmarks/src/engines/xerces/xerces_validation.cpp` (65)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Каждый движок: одинаковые настройки (пространства имён, сущности, проверка) или различия явно оговорены в отчёте.
  2. Освобождение ресурсов каждой библиотеки (Xerces Terminate, libxml2 cleanup).
  3. `*_queries` / `*_traverse`: одинаковые запросы и обходы.
  4. RapidXML меняет входной буфер — копия делается в каждой итерации и учитывается одинаково.
  5. Ошибки разбора не игнорируются молча.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-52. Ревью: бенчмарки обёрток: Node.js, Python, C#

- **Зачем.** Сравнения обёрток с родными библиотеками своих языков. Риски — сборщик мусора и JIT искажают замер, разная работа у сравниваемых библиотек.
- **Приоритет:** когда будет время · ~50–90 минут (файлов: 23, строк: ≈ 1342 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-52/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `benchmarks/node/compare.js` (95)
  2. `benchmarks/node/engines/beresta.js` (57)
  3. `benchmarks/node/engines/fast_xml_parser.js` (53)
  4. `benchmarks/node/engines/index.js` (74)
  5. `benchmarks/node/engines/libxmljs.js` (53)
  6. `benchmarks/node/engines/xmldom.js` (63)
  7. `benchmarks/node/workload.js` (69)
  8. `benchmarks/wrappers/make_data.js` (32)
  9. `benchmarks/wrappers/python/bench.py` (92)
  10. `benchmarks/wrappers/python/engine_beresta.py` (42)
  11. `benchmarks/wrappers/python/engine_etree.py` (39)
  12. `benchmarks/wrappers/python/engine_lxml.py` (36)
  13. `benchmarks/wrappers/python/memory_probe.py` (37)
  14. `benchmarks/wrappers/python/stream_bench.py` (75)
  15. `benchmarks/wrappers/csharp/BerestaEngine.cs` (61)
  16. `benchmarks/wrappers/csharp/Engine.cs` (35)
  17. `benchmarks/wrappers/csharp/ErrorCounter.cs` (38)
  18. `benchmarks/wrappers/csharp/LinqXmlEngine.cs` (61)
  19. `benchmarks/wrappers/csharp/MemoryProbe.cs` (53)
  20. `benchmarks/wrappers/csharp/Program.cs` (114)
  21. `benchmarks/wrappers/csharp/StreamBench.cs` (72)
  22. `benchmarks/wrappers/csharp/SystemXmlEngine.cs` (68)
  23. `benchmarks/wrappers/csharp/WrapperBench.csproj` (23)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Прогрев JIT (Node, C#), сборка мусора между замерами.
  2. Одинаковая работа у сравниваемых библиотек.
  3. `memory_probe` / `MemoryProbe`: что именно меряется.
  4. `make_data.js`: детерминированные данные.
  5. Освобождение документов Берёсты в цикле замеров.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-53. Ревью: бенчмарки обёрток: Java и PHP

- **Зачем.** То же для Java и PHP.
- **Приоритет:** когда будет время · ~40–70 минут (файлов: 15, строк: ≈ 1004 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-53/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `benchmarks/wrappers/java/src/bench/BerestaEngine.java` (74)
  2. `benchmarks/wrappers/java/src/bench/CountingErrorHandler.java` (46)
  3. `benchmarks/wrappers/java/src/bench/Engine.java` (39)
  4. `benchmarks/wrappers/java/src/bench/JdkEngine.java` (87)
  5. `benchmarks/wrappers/java/src/bench/MemoryProbe.java` (57)
  6. `benchmarks/wrappers/java/src/bench/Operation.java` (13)
  7. `benchmarks/wrappers/java/src/bench/StreamBench.java` (89)
  8. `benchmarks/wrappers/java/src/bench/WrapperBench.java` (102)
  9. `benchmarks/wrappers/php/BerestaEngine.php` (72)
  10. `benchmarks/wrappers/php/DomEngine.php` (70)
  11. `benchmarks/wrappers/php/Engine.php` (49)
  12. `benchmarks/wrappers/php/Measure.php` (82)
  13. `benchmarks/wrappers/php/SimpleXmlEngine.php` (53)
  14. `benchmarks/wrappers/php/bench.php` (78)
  15. `benchmarks/wrappers/php/stream_bench.php` (93)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Прогрев JIT (Java), opcache (PHP).
  2. Одинаковая работа у сравниваемых библиотек.
  3. `MemoryProbe` / `Measure`: что именно меряется.
  4. Освобождение документов Берёсты в цикле.
  5. Обработка ошибок разбора (`CountingErrorHandler`).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-54. Ревью: примеры C++ и Си

- **Зачем.** Примеры копируют пользователи. Риски — пример учит небезопасному (внешние ресурсы включены, ошибки не проверяются, утечки в Си-примере).
- **Приоритет:** когда будет время · ~20–30 минут (файлов: 9, строк: ≈ 483 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-54/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `examples/capi/capi_demo.c` (102)
  2. `examples/include/demo/demo_scenarios.hpp` (45)
  3. `examples/include/demo/element_counter.hpp` (36)
  4. `examples/include/platform/console.hpp` (11)
  5. `examples/src/demo/demo_advanced.cpp` (80)
  6. `examples/src/demo/demo_scenarios.cpp` (105)
  7. `examples/src/demo/element_counter.cpp` (44)
  8. `examples/src/main.cpp` (36)
  9. `examples/src/platform/console.cpp` (24)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Си-пример: каждый хэндл освобождается, каждый код ошибки проверяется.
  2. Примеры не включают загрузку внешних ресурсов без пояснения.
  3. Консоль: вывод кириллицы на Windows (`console.cpp`).
  4. Примеры собираются из CMake и Makefile.
  5. Соответствие примеров текущему API.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 
