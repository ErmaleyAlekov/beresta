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
- **Результат:** Утечка глобального языка при падении теста в languages.test.js:29
В languages.test.js:29–33 язык переключается на en и возвращается на ru без try/finally. Если document.close() выбросит исключение между переключениями, язык останется английским для следующего теста в этом файле.
Отсутствие сброса языка перед тестом в test_languages.py:32
В test_languages.py в начале файла не вызывается beresta.set_language("ru"), в отличие от test_document.py:12 и test_move.py:11. Если перед test_languages.py выполнится код или тест, оставивший язык отличным от "ru", test_unknown_language упадёт на assertEqual(beresta.language(), "ru") в test_languages.py:38.
Мусор во временной директории при провале проверок в document.test.js:116, streaming.test.js:61, DocumentTest.php:120 и SchemaTest.php:81
Удаление временных файлов и директорий в document.test.js:128, streaming.test.js:71, DocumentTest.php:123 и SchemaTest.php:85 не обёрнуто в finally, поэтому при падении любого assert строками выше созданные каталоги beresta-* и файлы в /tmp навсегда остаются на диске.
Ослабленная проверка типа исключения в document.test.js:90
В errors() для root.appendElement('1bad') на document.test.js:90 и document.test.js:100 проверяется только caught[1].code === 'INVALID_NAME', но не caught[1] instanceof beresta.BerestaError, в отличие от caught[0] на document.test.js:96. Если обёртка выбросит обычный Error с полем code, тест всё равно пройдёт.
Ослабленная проверка закрытого документа в test_document.py:63
В test_errors() на test_document.py:63-64 конструкция with self.assertRaises(RuntimeError): root.text перехватывает любой RuntimeError без проверки текста сообщения (в отличие от test_languages.py:30), поэтому посторонний сбой рантайма внутри геттера text будет засчитан как успешное прохождение теста.
Жёстко зашитая версия библиотеки в test_xpath_schema.py:55
В test_xpath_schema.py:55 ожидаемый номер версии ядра задан строковым литералом. При смене версии тест упадёт.
Потеря детализации при несовпадении ожидаемого типа исключения в TestCase.php:55
В assertThrows() на TestCase.php:54-56 при перехвате неверного типа исключения не вызывается $this->fail(...) с сохранением исходного исключения как $previous, а TestRunner.php:57 ловит AssertionFailed и выводит место создания AssertionFailed вместо места исходного исключения.
Символы за пределами BMP (эмодзи / 4-байтовый UTF-8) не покрыты ни в одном из 23 файлов
Во всех тестах на не-ASCII строки (document.test.js:24, DocumentTest.php:27, test_document.py:21, wasm.test.mjs:43) используются только двухбайтовые в UTF-8 символы кириллицы, а суррогатные пары UTF-16 (например, 🌲) отсутствуют во всех четырёх обёртках.
Пробелы в покрытии путей ошибок и освобождения в PHP и Wasm относительно Node.js и Python
В DocumentTest.php и wasm.test.mjs отсутствует тест на двойной вызов close(), в wasm.test.mjs:75-89 нет проверки BerestaClosedError при обращении к узлу после document.close(), в SchemaTest.php нет проверки IO_ERROR при передаче несуществующего файла в validateFile() (есть в streaming.test.js:68 и test_streaming.py:55), а в MoveTest.php нет проверки запрещённого переноса узла в своего потомка (HIERARCHY_VIOLATION) и закрытия целевого документа target до обращения к перенесённому узлу (есть в move.test.js:42 и test_move.py:39)

## Разбор

Проверки Node.js и Python теперь задают язык теста и восстанавливают его при завершении. Файловые тесты Node.js и PHP удаляют временные данные и при падении утверждения. Проверки различают тип и код ошибки, сверяют сообщение закрытого документа и принимают номер следующей версии библиотеки.

Добавлена проверка символа вне BMP во всех четырёх обёртках. PHP и Wasm проверяют повторное закрытие и обращение к закрытому объекту. В PHP проверяются ошибка чтения отсутствующего файла, запрещённый перенос в потомка и время жизни перенесённого узла. При неверном типе исключения PHP сохраняет исходную причину и выводит место ошибки. Ручное ревью обнаружило пути, которые прежние положительные тесты пропускали.

Прошли Node.js 21/21, WebAssembly 8/8 после пересборки Emscripten 3.1.69, Python 27/27 и PHP 40/40. Копии для ревью Т-36 удалены.

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
- **Результат:** LanguageChecks.java:25-32: Expect.equal внутри englishParseError() при провале бросает AssertionError (наследник Error), который проходит мимо catch (RuntimeException failure) на строке 28 прямо в SelfTest.run, оставляя глобальный язык библиотеки в состоянии "en" для всех последующих проверок; воспроизводится любым изменением текста английской ошибки разбора.
LoadChecks.cs:54 и ValidationWorker.cs:35-38: исключение внутри фонового потока ValidationWorker.Run() не перехватывается ни рабочим потоком, ни Program.Run(), что при сбое в нативной проверке или парсинге роняет весь процесс SelfTest с необработанным исключением потока вместо фиксации [FAIL].
LoadChecks.cs:49-53: потоки запускаются в цикле foreach (Thread thread in threads) thread.Start() без барьера старта. Проверка может пройти и при последовательном выполнении работы потоков.
DocumentChecks.cs:96-105 и FileChecks.java:21-33: удаление временного каталога Directory.Delete(folder, true) / Files.delete выполняется без блока finally, поэтому при провале любой из проверок кодировки или текста временные файлы и директории остаются на диске.
DocumentChecks.java:64: в комментарии к методу errors() заявлена проверка недопустимого имени (an invalid name), но в теле метода (DocumentChecks.java:66-94) проверяются только синтаксическая ошибка разбора и обращение к закрытому документу

## Разбор

В Java язык восстанавливается через finally, файловые тесты Java и C# освобождают временные каталоги и при ошибке. Java отдельно проверяет INVALID_NAME у недопустимого имени элемента.

Все потоки нагрузочной проверки C# доходят до общего старта. Основной поток дожидается их завершения и передаёт собранные исключения в отчёт SelfTest. Прежний тест мог успешно пройти при последовательном выполнении запросов или оборвать процесс при ошибке рабочего потока. Это обнаружилось при чтении тестового кода.

Прошли C# 16/16 и Java 21/21. Копии для ревью Т-37 удалены.

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
- **Результат:** tests/src/support/dom_helpers.cpp:41 в функции makeTestDirectory каталог создаётся по фиксированному пути temp_directory_path() / "beresta_tests" / name и удаляется только перед созданием, но не после теста. При одновременном запуске двух процессов тестов с одинаковым именем каталога они перезапишут или удалят файлы друг друга, а после прогона временные файлы останутся в системе.
tests/src/support/tree_helpers.cpp:20 в функции deepestElement указатель cursor = document.getRoot() сразу разыменовывается в цикле cursor->getFirstChild() без проверки на nullptr. Вызов deepestElement на пустом beresta::Document без корневого элемента приводит к разыменованию nullptr (неопределённое поведение).

## Разбор

Временные каталоги получили владельца `support::TestDirectory`. Каждый экземпляр атомарно создаёт отдельный каталог и удаляет его содержимое при выходе из области видимости. Регрессия проверяет два одновременно живущих каталога с одинаковым именем и удаление файла вместе с владельцем. Для пустого документа `deepestElement` возвращает `nullptr`. На прежнем последовательном прогоне конфликты каталогов и пустая цепочка не проверялись.

Копии для ревью Т-41 удалены.

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
- **Результат:** i18n_tests.cpp:55 и i18n_tests.cpp:74: глобальный язык переключается на английский через beresta::i18n::setLanguage("en"), а возврат на "ru" стоит в конце функции без RAII-обёртки. Если проверка прервёт тест до возврата на "ru", глобальный язык останется английским после выхода из функции.
capi_tests.cpp:11: в тестах Си-интерфейса полностью отсутствуют проверки на передачу nullptr в функции освобождения (beresta_document_free(nullptr), beresta_xpath_free(nullptr), beresta_schema_free(nullptr), beresta_nodeset_free(nullptr), beresta_diagnostics_free(nullptr)).
capi_tests.cpp:11: отсутствует проверка, что beresta_document_parse при превышении лимита глубины возвращает BERESTA_LIMIT_EXCEEDED и обнуляет выходной дескриптор.
capi_bulk_move_tests.cpp:115: проверка производительности TEST_ASSERT(elapsed.count() < 5.0) завязана на реальное настенное время (steady_clock). На нагруженном сборочном сервере или при прогоне под санитайзерами и Valgrind выполнение 150 000 операций со строками и дескрипторами может превысить порог в 5 секунд и вызвать ложное падение теста.
traversal_tests.cpp:17: в NameFilter::acceptNode выполняется безусловное приведение static_cast<const beresta::Element&>(node) без проверки node.getType(). Если этот фильтр передать в TreeWalker или NodeIterator с флагом beresta::show::ALL или beresta::show::TEXTS на документе с текстовым узлом, произойдёт чтение памяти Text как Element (неопределённое поведение).
traversal_tests.cpp:58: в тестах TreeWalker и NodeIterator отсутствует сценарий переноса живого текущего узла во время активного обхода.
encoding_tests.cpp:41: в testEncodingErrors не проверяются непарные суррогаты в UTF-16 (одинокий старший суррогат "\xFF\xFE\x00\xD8" или младший "\xFF\xFE\x00\xDC" без пары). При разборе повреждённого файла UTF-16LE с непарным суррогатом отсутствие ошибки ENCODING_ERROR останется незамеченным.
serializer_tests.cpp:36 и mapped_file_tests.cpp:30: не протестированы пути ошибок файлового ввода-вывода при записи и отображении. В serializer_tests.cpp нет проверки на запись в несуществующий каталог (writeFile на путь вида directory / "missing_dir" / "out.xml") и на неизвестную кодировку в options.encoding, а в mapped_file_tests.cpp нет проверки на передачу пути к директории вместо файла (beresta::io::MappedFile(directory)).
element_tests.cpp:51: макрос TEST_ASSERT_THROWS(beresta::Element("1bad"), beresta::DomException) проверяет только тип исключения, но не его код ошибки. Если конструктор Element или setAttribute выбросит DomException с кодом HIERARCHY_VIOLATION вместо ожидаемого по спецификации INVALID_NAME, тест всё равно пройдёт.

## Разбор

Тесты локализации восстанавливают исходный язык при выходе. Для Си-интерфейса проверяются пять вызовов `free(NULL)`, локализованное сообщение парсера и возврат `BERESTA_LIMIT_EXCEEDED` с пустым результатом при превышении глубины. Массовый перенос проверяет владение, порядок и доступ к узлам после освобождения исходного документа. Порог в пять секунд снят.

Фильтр обхода проверяет тип узла. Регрессия переносит живой текущий узел и продолжает обход через TreeWalker и NodeIterator. Повторное освобождение одного ненулевого дескриптора и чтение курсора после уничтожения узла выходят за контракт времени жизни. Добавлены точные коды ошибок для имён, суррогатов UTF-16, записи в отсутствующий каталог и неизвестной кодировки. Резервное файловое чтение выдаёт IO_ERROR для каталога. Эти пути отказа обнаружились при чтении тестов.

Копии для ревью Т-42 удалены.

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
- **Результат:** В tests/src/dtd/dtd_entity_tests.cpp:52 проверяются глубокое вложенное раскрытие и взаимная рекурсия, но отсутствует проверка лимита при многократном раскрытии одной большой сущности без вложенности. В tests/src/xpath/xpath_name_tests.cpp:15 и tests/src/xpath/xpath_path_tests.cpp:42 не проверяются выражения, где звёздочка служит и оператором умножения, и шаблоном узла (* * *, a/* * 2), а также ключевые слова в роли имён элементов (/div/mod/and/or). В tests/src/xpath/xpath_namespace_tests.cpp:74 порядок документа проверяется только между пространством имён и атрибутом одного элемента, но нет проверки порядка между атрибутами и дочерними или последующими элементами на входе вроде (/a/@x | /a/b) или (/a/b/@x | /a/c). В tests/src/parser/error_tests.cpp:8 нет проверок позиции ошибки после \r\n, внутри многострочных атрибутов (<a x="1\n  &bad;"/>) и внутри подключённых сущностей. В tests/src/parser/w3c_syntax_tests.cpp:66 утверждение errorCodeOf(document) != beresta::DomErrorCode::NOT_FOUND не проверяет конкретный код ошибки и пропустит подмену синтаксической ошибки на любой другой код. В tests/src/dtd/dtd_validation_tests.cpp:29 проверка errors >= 6 не фиксирует точное число ошибок валидации и не поймает дублирование одного и того же сообщения валидатором. В tests/src/xpath/xpath_path_tests.cpp:42 отсутствует проверка обратной оси ancestor::* с позиционным предикатом [1], который выбирает ближайшего предка, а также не проверяются граничные случаи функции substring с NaN, бесконечностью и начальной позицией 0 (substring("12345", 0, 3)).

## Разбор

Для повторных ссылок на плоскую сущность тест проверяет точное заполнение бюджета в четыре байта и код ошибки следующего раскрытия. Зафиксированы позиции при CRLF, внутри многострочного атрибута и при ошибке в подстановке сущности, конкретные коды синтаксических ошибок и число диагностик DTD.

XPath-проверки различают умножение и выбор узлов, имена элементов и операторы, проверяют порядок атрибутов относительно детей и соседей, ближайшего предка и границы substring. Обратная ось preceding уже проверялась с позиционными предикатами.

Копии для ревью Т-43 удалены.

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
- **Результат:** xsd_assert_parallel_tests.cpp:69: потоки запускаются в цикле без общего барьера старта, поэтому ранний поток может закончить проверку до запуска остальных.
xsd_regex_syntax_tests.cpp:1: отсутствуют проверки времени выполнения на неоднозначных повторах и верхней границы квантификатора.
xsd_datatype_tests.cpp:44 проверяет только отклонение QName с необъявленным префиксом p:name, но не проверяет валидный QName в контексте с объявленным пространством имён. Помощник support::acceptsBuiltin не задаёт контекст пространств имён.
xsd_datatype_tests.cpp:25 и xsd_duration_order_tests.cpp:36 пропускают граничные случаи из Т-19, такие как год 0000 (запрещённый в XSD 1.0 и разрешённый в 1.1), смещения часовых поясов на границах +14:00 / -14:00 и недопустимые +14:01, отрицательные значения самих границ minInclusive / maxInclusive и сравнение эквивалентных длительностей вроде PT24H и P1D.

## Разбор

Восемь рабочих потоков синхронизируют старт проверки общей схемы. Добавлены проверки QName с объявленным и неизвестным префиксом, года 0000 в профиле XSD 1.0, смещений ±14:00 и ±14:01, отрицательных границ длительностей и равенства PT24H и P1D. Для регулярных выражений проверены неоднозначный повтор, границы диапазона и числовой предел квантификатора при разборе. Эти тесты не измеряют время выполнения. Для фиксированного автомата линейная граница следует из отметки каждого состояния один раз на входной символ в `Regex::addReachable`.

У элемента с чисто элементным содержимым переменная `$value` содержит пустую последовательность. При попытке атомизации самого элемента возникает ошибка. Оба поведения проверены отдельно. Фасет `xs:assertion` над списком разрешён стандартом и выходит за документированное подмножество библиотеки. Тест проверяет обычный список и конкретную диагностику при добавлении фасета. [XSD 1.1 Structures, §3.13.4.1](https://www.w3.org/TR/xmlschema11-1/#cAssertions), [XSD 1.1 Datatypes, §4.3.13](https://www.w3.org/TR/xmlschema11-2/#rf-assertions), [профиль библиотеки](../../../docs/comparison.md).

Копии для ревью Т-44 удалены.

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
- **Результат:** xsd_deep_tests.cpp:104 Утечка ресурса Си при провале ассерта (О-3, О-5). В testDeepDocumentLimitInCapi вызывается beresta_schema_load, а освобождение beresta_schema_free(schema) стоит в самом конце функции. Если схема успешно загружена, а проверка результата beresta_schema_validate_bytes затем падает, исключение из TEST_ASSERT обходит beresta_schema_free(schema).
xsd_stream_tests.cpp:89 Потеря проверки согласованности состояния объекта после исключения (О-5). В testEventsPassThrough вызывается SaxParser(options).parse(ORDER_XML, strict) без errorHandler, что выбрасывает beresta::ParseException, но после этого тот же экземпляр options.schema больше не используется для валидации корректного документа. Если исключение оставило внутреннее состояние схемы или кэша несогласованным, это останется незамеченным.
xsd_redefine_rules_tests.cpp:74 Отсутствует тест на взаимные пустые xs:redefine. В testIncludeBackToRoot проверен только цикл через xs:include, а случай, когда a.xsd переопределяет b.xsd, а b.xsd переопределяет a.xsd, не покрыт. Воспроизводится загрузкой a.xsd с <xs:redefine schemaLocation="b.xsd"/> при обратном xs:redefine из b.xsd на a.xsd.
xsd_stream_identity_tests.cpp:85 Нет проверки сброса состояния валидатора между документами (шаг ревью по потоку). В testHandlerBeforeDomBuilder экземпляр ValidatingHandler создаётся на один вызов parse и нигде в наборе тестов не переиспользуется для второго документа подряд. Воспроизводится повторным вызовом SaxParser::parse с тем же ValidatingHandler для валидного документа после документа с ошибкой ключа или открытым IDREF.
xsd_identity_tests.cpp:58 В testCompositeKeyRules и в xsd_identity_scope_tests.cpp:21 (testKeyFromDescendantScope) используется support::schemaErrors вместо support::agreedSchemaErrors, поэтому именно в этих двух сценариях диагностики SAX и проверки дерева не сравниваются.

## Разбор

Ручное ревью выявило, что повторный разбор через один ValidatingHandler наследовал состояние предыдущего документа. При новом startDocument обработчик начинает проверку с чистыми таблицами и с локатором нового документа. Регрессии повторно используют обработчик после ошибки ключа и висячего IDREF, а также используют ту же схему после исключения строгого SAX-разбора.

Схема и диагностика Си-интерфейса в тесте глубины освобождаются через RAII. Взаимные пустые xs:redefine проверяются с корректным и ошибочным документами. Для составного ключа и ссылки на ключ в области-потомке тесты сравнивают диагностики SAX, разбора в DOM и проверки готового дерева.

Общая проверка Т-41–Т-45: 276/276 модульных тестов. W3C XML Conformance: 2 001/2 001 применимых, 584 пропуска. XSTS: 452 известных расхождения среди 39 420 случаев. Повторный прогон по списку из 515 прежних провалов подтвердил те же 452 идентификатора.

Копии для ревью Т-45 удалены.

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
- **Результат:** tests/conformance/src/main.cpp:124 — при наличии проваленных тестов программа всё равно возвращает код 0 вместо ненулевого значения, из-за чего CI не сможет обнаружить падение прогона. Воспроизводится запуском conformance_runner на каталоге, где хотя бы один тест завершается с Verdict::FAILED.
tests/conformance/src/catalog/catalog_fields.cpp:54 — поиск файла подкаталога по голой подстроке "\"" + id + "\"" находит чужой XML-файл, если ID теста совпадает со значением любого другого атрибута (например, ID="valid" совпадёт с TYPE="valid" в первом же файле). Воспроизводится тестом с несуществующим первичным путём и значением ID, встречающимся в атрибутах другого подкаталога.
tests/conformance/src/canonical/canonical_escape.cpp:11 — функция skipLeadingInstructions останавливается сразу после ?> и не пропускает переводы строк или пробелы перед <!DOCTYPE, из-за чего stripCanonicalDoctype не вырезает <!DOCTYPE во второй канонической форме. Воспроизводится эталонным файлом OUTPUT формата <?pi data?>\n<!DOCTYPE doc [...]>\n<doc></doc>.
tests/conformance/src/main.cpp:47 — при передаче флага --report или --only без значения в конце командной строки условие hasValue становится ложным и код попадает в ветку «неизвестный аргумент: --report», а передача двух позиционных путей молча перезаписывает первый. Воспроизводится запуском conformance_runner --report или conformance_runner dir1 dir2.
tests/conformance/src/runner/case_checks.cpp:52 — в skipReason проверяется существование только testCase.file, но не проверяется testCase.output, поэтому при отсутствии файла OUTPUT на диске compareOutput выбрасывает ConformanceError и аварийно обрывает весь прогон вместо пропуска или провала одного теста. Воспроизводится тестом с TYPE="valid", существующим входным файлом и отсутствующим файлом по пути из атрибута OUTPUT

## Разбор

Прогон возвращает 1 при расхождении и 2 при ошибке запуска. Отсутствующий OUTPUT помечает провал одного случая. Для восстановления базы внешнего подкаталога используются ID элементов TEST, объявленные в контексте DTD каталога. Сравнение второй канонической формы сохраняет инструкции обработки и убирает разделяющие их пробелы пролога. Аргументы, отсутствие эталона, совпадение ID со значением чужого атрибута и PI перед DOCTYPE проверяются в `tests/tools/test_runner_cli.py`.

Полный W3C XML Conformance прошёл 2 001 из 2 001 применимого случая, ещё 584 случая пропущены.

Копии для ревью Т-46 удалены.

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
- **Результат:** В fuzz_schema.cpp:58 в функцию validateWhileParsing не передаётся флаг namespaceAware, поэтому режим namespaceAware = false (выставленный байтом флагов 0x01) вообще не фаззится для встроенной в парсер валидации, хотя для отдельного валидатора и SAX он проверяется; воспроизводится любым входом fuzz_schema с нечётным первым байтом.
В fuzz_schema.cpp:26 основная схема и ресурс MemoryResolver разбираются со стандартными лимитами парсера, хотя для других входов фаззера в optionsFromFlags заданы уменьшенные лимиты.
В fuzz_xpath.cpp:36 функция loadDocument создаёт XmlParser со стандартными лимитами вместо уменьшенных лимитов из optionsFromFlags.
В cpp_literals.py:48 поиск строковых литералов регулярным выражением срабатывает на символьный литерал '"' в коде C++, из-за чего последующие строковые литералы могут извлекаться неверно.
В make_corpus.py:72 при формировании корпуса fuzz_schema схемы ищутся строго по подстроке <xs:, поэтому схемы с другим префиксом (например, xsd:) или с пространством имён XSD по умолчанию пропускаются.

## Разбор

Для XPath и трёх вариантов проверки документа по схеме установлены сокращённые лимиты разбора. Режим пространств имён передаётся также встроенному валидатору. Основная схема и содержимое ресурса из памяти проходят ограниченный разбор. Извлечение строк C++ учитывает символьные литералы и комментарии. Полные схемы для корпуса распознаются по URI пространства имён. Обе регрессии инструментов корпуса проходят в `tests/tools/test_corpus_helpers.py`.

Копии для ревью Т-47 удалены.

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
- **Результат:** catalog_case.cpp:15 (readExpectation): функция выбирает первое ожидание с известным значением validity и игнорирует атрибут version. Если ожидание для XSD 1.1 стоит перед отличающимся ожиданием для XSD 1.0, прогон XSD 1.0 сравнит результат с ожиданием для другой версии.
catalog_loader.cpp:74 и catalog_case.cpp:69 (loadTestSet, makeCase): атрибут version считывается исключительно с самого элемента schemaTest или instanceTest, хотя по схеме XSTS он наследуется от родительских элементов testSet и testGroup. Если поместить группу тестов с <testGroup version="1.1">, она не будет пропущена в skipReason и пойдёт на прогон как XSD 1.0.
schema_hints.cpp:62 (hintedDocuments) и catalog_case.cpp:56 (readDocuments): относительные пути из xsi:schemaLocation и xlink:href нормализуются через lexically_normal(), но не проверяются на выход за пределы suiteRoot. В результате detectFeatures, targetNamespaces и driverText читают произвольные файлы вне каталога набора через readFile и XmlParser().parseFile в обход FileResourceResolver.
group_runner.cpp:25 (GroupRunner::run) и instance_validation.cpp:94 (validateInstance): отсутствует ограничение времени выполнения на отдельный тестовый случай.
report_lists.cpp:25 (oneLine): обрезка текста выполняется жёстко через line.substr(0, LIMIT) на 400-м байте без проверки байтов продолжения UTF-8 (0x80..0xBF), в отличие от groupDescription в catalog_fields.cpp:65. Если кириллическое сообщение об ошибке превысит 400 байт и 400-й байт попадёт в середину двухбайтового символа, в выходной файл xsts_report.md запишется невалидный UTF-8.
main.cpp:95 (main): программа всегда возвращает код 0, даже если в результате прогона обнаружены расхождения (summary.total.failed > 0). При запуске прогонщика из скрипта или CI падение тестов не приводит к ненулевому коду возврата.

## Разбор

Прогон для XSD 1.0 выбирает версионное ожидание с резервом на общее и учитывает атрибуты testSet и testGroup. Метаданные, ссылки документов и подсказки схем проверяются относительно корня набора с разрешением символьных ссылок. Ошибки чтения документов сохраняются с конкретным случаем. Обе точки разбора из замечания уже находились внутри обработчиков исключений. Отчёт обрезает строки по границе UTF-8, код 1 означает расхождение. Сторожевой поток ограничивает отдельный случай 60 секундами. После тайм-аута или аварии управляющий процесс повторяет проход, записывая отказавшие случаи в итоговый отчёт.

Полный XSTS обработал 39 420 случаев. Прошли 37 679, выявлены 452 расхождения, 412 отнесены к ограничениям и 877 пропущены. Процесс вернул 1. По сравнению с версионированным отчётом и TSV исчезли 63 из 515 прежних идентификаторов расхождений, новых нет. Восстановление при аварии и тайм-ауте проверяет `xsts_worker_recovery`.

Копии для ревью Т-48 удалены.

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
- **Результат:** src/cli/options.cpp:74 — переполнение std::size_t при разборе номера случая в parseNumber: Вычисление number = number * 10 + static_cast<std::size_t>(symbol - '0') не проверяет выход за std::numeric_limits<std::size_t>::max(), из-за чего длинное число обёртывается по модулю и может совпасть с существующим номером записи вместо ошибки. Воспроизводится запуском xsts_failing --case 18446744073709551617, который выполнит случай под номером 1.
src/cli/options.cpp:25 — некорректная диагностика при передаче флага со значением последним аргументом без самого значения: Если --case, --list, --suite или --limitations стоит в конце командной строки, условие index + 1 < argc ложно, и код выбрасывает неизвестный аргумент вместо ошибки об отсутствии значения. Воспроизводится запуском xsts_failing --case.

## Разбор

Разбор номера проверяет переполнение до умножения и сложения, опции со значением диагностируют его отсутствие. Загрузчик уже проверял пустой список документов. Теперь отладчик также обрабатывает ошибку входа до обращения к первому документу. При `--regenerate` используется та же изоляция, что и у полного XSTS, а фактический результат попадает в TSV из одного прогона. Режим пошаговой отладки `--case` сохраняет обычную точку останова.

Копии для ревью Т-49 удалены.
