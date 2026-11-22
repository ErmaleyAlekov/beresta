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
- **Результат:** options.cpp:35 и measure.cpp:45 при передаче --runs 0 вектор values остаётся пустым, и вычисление медианы values[values.size() / 2] обращается по нулевому индексу к пустому буферу (неопределённое поведение). Воспроизводится запуском beresta_bench --runs 0.
options.cpp:11 и main.cpp:80 передача отрицательного индекса --memory -1 catalog не отклоняется при разборе аргументов и устанавливает memoryEngine = -1, из-за чего в main.cpp условие options.memoryEngine >= 0 не срабатывает и вместо замера памяти запускается весь бенчмарк. Воспроизводится вызовом beresta_bench --memory -1 catalog.
options.cpp:11,35-38 функции std::stol и std::stoul принимают строки с нечисловым хвостом вроде 10abc, молча отбрасывая суффикс, при отрицательных числах вроде --runs -1 преобразуют значение в огромное беззнаковое число вместо ошибки, а на нечисловом вводе выбрасывают std::invalid_argument вместо BenchError. Воспроизводится запуском beresta_bench --runs -1 или beresta_bench --items 10abc.
main.cpp:83-85 если путь из ключа --report указывает в несуществующую директорию или защищён от записи, report.open(options.report) молча не открывает поток, бенчмарк отрабатывает до конца, а отчёт не сохраняется без какой-либо диагностики. Воспроизводится запуском beresta_bench --report /no_such_dir/out.md.
memory.cpp:58-63 путь к исполняемому файлу program подставляется в std::system() без экранирования внутренних кавычек, и на не-Windows платформах substr(1, command.size() - 2) оставляет только внешнюю пару кавычек вокруг пути, из-за чего кавычка в имени каталога ломает команду оболочки и возвращает 0.0 в замер памяти. Воспроизводится запуском программы из директории с символом " в пути.
memory.cpp:41-46 функция residentBytes() читает текущие резидентные страницы (/proc/self/statm на Linux и WorkingSetSize на Windows), а не пиковое потребление (VmHWM или PeakWorkingSetSize), а на macOS вовсе возвращает 0.0, поэтому временные выделения памяти внутри parse() не учитываются, а на macOS столбец памяти всегда пустой. Воспроизводится любым запуском на macOS или сравнением парсера с большими временными буферами во время разбора.
report_checks.cpp:25-33 в таблице сверки результатов отсутствует столбец для операции STREAM_VALIDATE (m[5]), хотя она замеряется в measure.cpp:35 и выводится в основную таблицу в report.cpp:62, а сама функция только печатает числа и не проверяет совпадение контрольных сумм и счётчиков между движками. Воспроизводится любым прогоном с поддержкой XSD, где проверка streamValidated вернёт расходящееся число ошибок.

## Разбор

Аргументы проверяются целиком и в диапазоне целевого типа. Нулевое число замеров и ошибки записи отчёта дают ненулевой код. Измерение памяти запускает процесс с отдельными аргументами и собственным временным каталогом. В таблице отдельно показаны прирост резидентной памяти после разбора и прирост максимального RSS за разбор. Сверка включает потоковую валидацию и сообщает о расхождении поддерживаемых операций. Размер сериализации остаётся справочной величиной, поскольку движки по-разному форматируют эквивалентный XML.

Копии для ревью Т-50 удалены.

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
- **Результат:** pugixml_engine.cpp:16 при разборе с флагами по умолчанию parse_default отбрасывает чисто пробельные текстовые узлы (parse_ws_pcdata не передан). На форматированном XML с отступами эти узлы не попадают в контрольную сумму обхода.
libxml2_engine.cpp:25, libxml2_queries.cpp:36 и libxml2_stream.cpp:17, 36 сужают длину входного буфера до int без проверки на INT_MAX. Длина больше INT_MAX не представима в параметре C API.
libxml2_queries.cpp:36–40 не проверяет результат xmlReadMemory на nullptr, передаёт его в xmlSchemaValidateDoc и игнорирует код возврата. В результате вызывающий код получает счётчик диагностик даже при ошибке разбора.
xerces_validation.cpp:23–27 в методе ErrorCounter::fatalError инкрементирует счётчик ошибок валидации и не выбрасывает исключение, поэтому на синтаксически битом XML (например, "<root>") XercesEngine::parseValidated вернёт количество синтаксических ошибок под видом ошибок XSD-валидации, тогда как BerestaEngine::parseValidated в beresta_queries.cpp:35 выбросит исключение при разборе.
rapidxml_engine.cpp:19 вызывает _document.parse<0> без флага rapidxml::parse_declaration_node, отбрасывая заголовок <?xml version="1.0"?>, из-за чего RapidxmlEngine::serialize() в rapidxml_output.cpp:13 выведет строку меньшего размера без XML-декларации даже на документах, где декларация присутствовала во входе.
libxml2_engine.cpp:24 и xerces_engine.cpp:44 вызывают release() до завершения разбора. При ошибке прежнее дерево потеряно, _document остаётся равным nullptr, а traverse() обращается к нему без проверки.
xerces_traverse.cpp:45–47 освобождает output и serializer ручными вызовами release() после serializer->write(...) без RAII-обёртки, что приведёт к утечке памяти обоих объектов при выбросе исключения xercesc::XMLException или xercesc::DOMException во время сериализации.

## Разбор

pugixml сохраняет пробельные узлы, RapidXML сохраняет XML-декларацию. libxml2 проверяет представимость длин и результаты выделения контекстов и валидации. libxml2 и Xerces сохраняют предыдущее дерево при неудачном разборе. Xerces отличает фатальную ошибку синтаксиса от ошибок схемы и освобождает объекты сериализации через RAII. CTest `bench_engine_checks` проверяет адаптеры, подключённые к сборке.

Копии для ревью Т-51 удалены.

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
- **Результат:** compare.js:35-44 при нескольких итерациях medianTime каждый вызов parse перезаписывает предыдущий результат. В measureEngine на строке compare.js:68 engine.release вызывается только для последнего документа.
engine_beresta.py:42, engine_lxml.py:39 и BerestaEngine.cs:53 при нарушении XSD во время DOM-разбора выбрасывают исключение, которое прерывает бенчмарк. SystemXmlEngine и LinqXmlEngine собирают ошибки через ErrorCounter. В потоковом бенчмарке Python необработанное исключение при нарушении схемы возникает в lxml_once.
Program.cs:37 и bench.py:67 в режиме --memory индекс движка из аргумента командной строки передаётся напрямую в индексацию массива engines[int.Parse(args[2])] и списка available_engines(...)[int(sys.argv[3])] без проверки на число и выход за границы массива. Воспроизводится запуском dotnet run -- data --memory 99 или python bench.py data --memory 99, что приводит к необработанному исключению выхода за границы коллекции.

## Разбор

В Node.js каждый промежуточный документ освобождается после остановки таймера, последний живёт до конца обхода и XPath. Это проверяют два теста в `benchmarks/node/compare.test.js`. Python закрывает результаты при исключениях. Python и C# отклоняют ошибочный индекс движка. DOM- и потоковые отчёты показывают признак валидности XSD отдельно от количества диагностик. Валидный и невалидный документы проверены в обоих режимах. Повторяемая проверка таблиц и ошибочного индекса находится в `benchmarks/tests/wrapper_checks.py`.

Копии для ревью Т-52 удалены.

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
- **Результат:** BerestaEngine.java:63 и JdkEngine.java:74 при валидации выполняют разную работу. BerestaEngine.parseValidated оборачивает разбор в try (XmlDocument document = ...), включая время нативного close() в замер, тогда как JdkEngine.parseValidated оставляет освобождение дерева сборщику мусора. Также при наличии ошибок XSD BerestaEngine выбрасывает исключение на первом же нарушении, а JdkEngine через CountingErrorHandler продолжает парсить и считать все ошибки до конца файла. При нарушении схемы исключение BerestaEngine прерывает вывод таблицы WrapperBench.
 BerestaEngine.php:58 и DomEngine.php:62 аналогично расходятся в поведении и работе. BerestaEngine::parseValidated не ловит исключение валидации и прерывается на первой ошибке, а DomEngine::parseValidated дочитывает файл и возвращает число диагностик из libxml_get_errors(). На невалидном XML bench.php аварийно завершится с неперехваченным исключением.
 DomEngine.php:48 на каждом прогоне XPath заново создаёт объект new \DOMXPath($document), в то время как BerestaEngine.php:51 переиспользует заранее скомпилированное выражение new XPath($query) из конструктора. В замер XPath для ext/dom каждый раз попадают накладные расходы на инициализацию и уничтожение объекта DOMXPath.
 StreamBench.java:81 внутри измеряемой функции once на каждом прогоне создаёт новый Validator (jdk.newValidator()) и CountingErrorHandler, тогда как для «Берёсты» используется готовый XmlSchema. Измерение скорости потоковой валидации JDK загрязняется временем инстанцирования валидатора.
 WrapperBench.java:35 и bench.php:63 не проверяют индекс движка для --memory. Например, java bench.WrapperBench data --memory 99 выбросит ArrayIndexOutOfBoundsException. В PHP bench.php и stream_bench.php приводят результат неудачного file_get_contents к пустой строке и продолжают работу.
 Measure.php:37, WrapperBench.java:64 и StreamBench.java:53 делят объём данных в мегабайтах на измеренное время в миллисекундах без защиты от нуля. На крошечном XML, который разберётся быстрее разрешения таймера (0.0 мс), вычисление скорости приведёт к делению на ноль (DivisionByZeroError в PHP или Infinity в Java).
 stream_bench.php:34 в функции readerErrors не оборачивает работу с libxml_use_internal_errors(true) в блок try ... finally. При возникновении исключения или сбое чтения глобальное состояние обработки ошибок libxml останется изменённым до конца процесса.

## Разбор

Java освобождает нативный валидированный документ после остановки таймера, а потоковый валидатор JDK переиспользуется. PHP переиспользует DOMXPath для текущего документа и восстанавливает состояние libxml при выходе с ошибкой. Исправлены граничные случаи с аргументами, путями и нулевой длительностью замера. В Java и PHP проверены валидный и невалидный XML, форма обеих таблиц и ошибочные аргументы. При одинаковом признаке валидности количество диагностик у движков может различаться.

Копии для ревью Т-53 удалены.

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
- **Результат:** В файле capi_demo.c на строке 55 результат вызова beresta_xpath_set_string(xpath, "code", "B2") не проверяется. На строках 69–71 также игнорируются результаты beresta_node_append_element, beresta_element_set_attribute и beresta_node_append_text. При отказе этих вызовов программа продолжает сценарий вместо вызова report_failure.
В файле main.cpp на строке 30 перехватывается только исключение beresta::DomException. Если внутри сценария возникнет std::bad_alloc, исключение выйдет из main и программа завершится через std::terminate.

## Разбор

Пример C проверяет дескрипторы и результаты навигации, привязки XPath, изменения дерева и сериализации. Диагностика печатается до освобождения ресурсов. C++ перехватывает `std::exception`. Утверждение о наследовании из карточки опровергнуто: `ParseException` уже является наследником `DomException`. Оба примера собраны и запущены. В `tests/tools/test_capi_demo.py` проверены два отказа с INVALID_ARGUMENT и код выхода 1.

Копии для ревью Т-54 удалены.
