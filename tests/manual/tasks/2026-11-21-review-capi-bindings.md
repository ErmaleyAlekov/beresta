# Ревью кода: Си-интерфейс и обёртки, 2026-11-21

Задачи ревью кода по правилу «человек в контуре» (`human_review.md`, раздел 3). Общие пункты О-1…О-8 и порядок работы — в [`../README.md`](../README.md).

## Т-25. Ревью: Си-интерфейс

- **Зачем.** Граница между C++ и всеми обёртками: хэндлы, владение, строки, коды ошибок. Исключение C++, прошедшее через Си-функцию, — неопределённое поведение; хэндл, освобождённый дважды, — порча памяти в любом языке.
- **Приоритет:** желательно до следующего выпуска · ~100–170 минут (файлов: 27, строк: ≈ 2523 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-25/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `include/capi/beresta_c.h` (541)
  2. `include/capi/internal/capi_support.hpp` (249)
  3. `include/beresta.hpp` (66)
  4. `src/capi/document/capi_document.cpp` (90)
  5. `src/capi/document/capi_document_io.cpp` (95)
  6. `src/capi/element/capi_element.cpp` (66)
  7. `src/capi/element/capi_element_details.cpp` (51)
  8. `src/capi/errors/capi_argument_exception.cpp` (16)
  9. `src/capi/errors/capi_error_record.cpp` (105)
  10. `src/capi/errors/capi_error_state.cpp` (44)
  11. `src/capi/handles/capi_handles.cpp` (70)
  12. `src/capi/library/capi_language.cpp` (93)
  13. `src/capi/library/capi_library.cpp` (76)
  14. `src/capi/library/capi_options.cpp` (66)
  15. `src/capi/node/capi_node_append.cpp` (60)
  16. `src/capi/node/capi_node_append_special.cpp` (30)
  17. `src/capi/node/capi_node_edit.cpp` (85)
  18. `src/capi/node/capi_node_info.cpp` (79)
  19. `src/capi/node/capi_node_move.cpp` (103)
  20. `src/capi/node/capi_node_navigation.cpp` (60)
  21. `src/capi/node/capi_node_relatives.cpp` (46)
  22. `src/capi/schema/capi_diagnostics.cpp` (47)
  23. `src/capi/schema/capi_schema.cpp` (73)
  24. `src/capi/schema/capi_schema_stream.cpp` (111)
  25. `src/capi/xpath/capi_nodeset.cpp` (57)
  26. `src/capi/xpath/capi_xpath.cpp` (72)
  27. `src/capi/xpath/capi_xpath_eval.cpp` (72)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. Каждая `extern "C"` функция: ловит ли все исключения (`catch (...)`) и превращает в код ошибки; проверяет ли `NULL` у каждого указателя-аргумента.
  2. `capi_handles` / владение: кто освобождает узел, документ, набор узлов; хэндл узла после освобождения документа; двойное освобождение.
  3. Строки: длина + указатель или нуль-терминированные — единообразно; время жизни возвращённых строк (до следующего вызова? до освобождения документа?).
  4. `capi_error_state`: ошибка хранится в потоке (thread_local) или глобально — гонка между потоками.
  5. `beresta_c.h`: только Си (никаких C++-типов), `extern "C"` под `__cplusplus`, экспорт/видимость символов на всех ОС, стабильность ABI (размеры перечислений).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-26. Ревью: обёртка Node.js: нативный аддон

- **Зачем.** C++-аддон N-API, загружающий функции N-API динамически (своя таблица символов). Риски — неверная ABI-таблица, исключения через границу N-API, утечки хэндлов и строк при ошибке посреди вызова.
- **Приоритет:** желательно до следующего выпуска · ~70–120 минут (файлов: 26, строк: ≈ 1851 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-26/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/node/addon/include/addon/adapters.hpp` (106)
  2. `bindings/node/addon/include/addon/call_frame.hpp` (125)
  3. `bindings/node/addon/include/addon/errors.hpp` (45)
  4. `bindings/node/addon/include/addon/napi_abi.hpp` (128)
  5. `bindings/node/addon/include/addon/napi_symbols.hpp` (29)
  6. `bindings/node/addon/include/addon/results.hpp` (52)
  7. `bindings/node/addon/src/adapters/adapters_handles.cpp` (74)
  8. `bindings/node/addon/src/adapters/adapters_numbers.cpp` (77)
  9. `bindings/node/addon/src/adapters/adapters_special.cpp` (91)
  10. `bindings/node/addon/src/adapters/adapters_status.cpp` (72)
  11. `bindings/node/addon/src/adapters/adapters_status_pairs.cpp` (20)
  12. `bindings/node/addon/src/adapters/adapters_strings.cpp` (72)
  13. `bindings/node/addon/src/adapters/adapters_text.cpp` (70)
  14. `bindings/node/addon/src/frame/arg_string.cpp` (52)
  15. `bindings/node/addon/src/frame/call_frame.cpp` (58)
  16. `bindings/node/addon/src/frame/call_frame_bytes.cpp` (38)
  17. `bindings/node/addon/src/frame/call_frame_values.cpp` (81)
  18. `bindings/node/addon/src/module/addon_state.cpp` (83)
  19. `bindings/node/addon/src/module/binding_table.cpp` (112)
  20. `bindings/node/addon/src/module/errors.cpp` (76)
  21. `bindings/node/addon/src/module/module.cpp` (80)
  22. `bindings/node/addon/src/napi/napi_loader.cpp` (85)
  23. `bindings/node/addon/src/napi/napi_symbols.cpp` (74)
  24. `bindings/node/addon/src/results/results_handles.cpp` (59)
  25. `bindings/node/addon/src/results/results_numbers.cpp` (41)
  26. `bindings/node/addon/src/results/results_strings.cpp` (51)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `napi_loader` / `napi_symbols` / `napi_abi.hpp`: сигнатуры совпадают с заголовками Node; что будет, если символ не найден.
  2. `call_frame*` / `arg_string`: число аргументов, тип каждого проверяется до чтения; строки UTF-16/UTF-8, длина > 2 ГБ.
  3. `adapters_*` / `results_*`: каждый код ошибки Си-интерфейса превращается в JS-исключение; хэндлы освобождаются на пути ошибки.
  4. `addon_state`: несколько экземпляров аддона (worker_threads) — общее состояние?
  5. `binding_table`: каждая запись таблицы указывает на верную функцию (опечатка — вызов не той функции).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-27. Ревью: обёртка Node.js: библиотека JS — ядро и узлы

- **Зачем.** JS-слой поверх аддона: фабрика узлов, реестр обёрток, освобождение (FinalizationRegistry), типы TypeScript. Риски — объект-обёртка, переживший документ, рассинхрон `index.d.ts` с реальным API.
- **Приоритет:** когда будет время · ~60–100 минут (файлов: 25, строк: ≈ 1447 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-27/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/node/lib/errors.js` (59)
  2. `bindings/node/lib/factory.js` (50)
  3. `bindings/node/lib/flags.js` (49)
  4. `bindings/node/lib/index.d.ts` (228)
  5. `bindings/node/lib/index.js` (38)
  6. `bindings/node/lib/languages.js` (70)
  7. `bindings/node/lib/internal/bytes.js` (35)
  8. `bindings/node/lib/internal/keys.js` (24)
  9. `bindings/node/lib/internal/mixin.js` (30)
  10. `bindings/node/lib/internal/native.js` (83)
  11. `bindings/node/lib/internal/release.js` (44)
  12. `bindings/node/lib/nodes/document.js` (57)
  13. `bindings/node/lib/nodes/document_content.js` (48)
  14. `bindings/node/lib/nodes/element.js` (55)
  15. `bindings/node/lib/nodes/element_attributes.js` (38)
  16. `bindings/node/lib/nodes/node.js` (25)
  17. `bindings/node/lib/nodes/node_base.js` (58)
  18. `bindings/node/lib/nodes/node_children.js` (49)
  19. `bindings/node/lib/nodes/node_content.js` (47)
  20. `bindings/node/lib/nodes/node_editing.js` (48)
  21. `bindings/node/lib/nodes/node_navigation.js` (46)
  22. `bindings/node/lib/nodes/node_structure.js` (51)
  23. `bindings/node/lib/nodes/registry.js` (121)
  24. `bindings/node/lib/nodes/wrapping.js` (55)
  25. `bindings/node/package.json` (39)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `internal/release.js` / `nodes/registry.js`: обёртка узла после `dispose()` документа — ошибка, а не падение; повторный `dispose`.
  2. `nodes/wrapping.js` / `factory.js`: один нативный узел → одна JS-обёртка (идентичность `===`).
  3. `internal/bytes.js`: Buffer/Uint8Array/строка, смещения и длины.
  4. `index.d.ts`: сверить с экспортами `index.js` (методы, необязательные параметры, типы возврата).
  5. `package.json`: `files`, `engines`, точки входа, что попадает в пакет.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-28. Ревью: обёртки Node.js (схемы, XPath) и WebAssembly

- **Зачем.** Схемы и XPath в JS и пакет `beresta-wasm` (сборщик, рантайм, примеры). Риски — память WASM (рост, освобождение), копирование строк в линейную память и обратно, расхождение API двух пакетов.
- **Приоритет:** когда будет время · ~50–90 минут (файлов: 20, строк: ≈ 1366 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-28/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/node/lib/schema/diagnostics.js` (50)
  2. `bindings/node/lib/schema/schema.js` (66)
  3. `bindings/node/lib/schema/schema_lifecycle.js` (35)
  4. `bindings/node/lib/schema/schema_streaming.js` (45)
  5. `bindings/node/lib/xpath/xpath.js` (68)
  6. `bindings/node/lib/xpath/xpath_context.js` (33)
  7. `bindings/node/lib/xpath/xpath_evaluation.js` (69)
  8. `bindings/node/lib/xpath/xpath_scalars.js` (34)
  9. `bindings/node/examples/demo.mts` (49)
  10. `bindings/wasm/lib/native_wasm.js` (89)
  11. `bindings/wasm/lib/wasm_call.js` (119)
  12. `bindings/wasm/lib/wasm_results.js` (90)
  13. `bindings/wasm/lib/wasm_scope.js` (72)
  14. `bindings/wasm/lib/wasm_signatures.js` (121)
  15. `bindings/wasm/tools/beresta.d.mts.txt` (32)
  16. `bindings/wasm/tools/bundle.mjs` (118)
  17. `bindings/wasm/tools/bundle_runtime.mjs.txt` (48)
  18. `bindings/wasm/examples/browser.html` (114)
  19. `bindings/wasm/package.json` (39)
  20. `bindings/wasm/exports.json` (75)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `schema_streaming.js`: потоковая проверка — освобождение при ошибке и при прерывании.
  2. `xpath_context.js` / `xpath_scalars.js`: переменные и пространства имён, типы скаляров (number/string/boolean).
  3. `bindings/wasm/lib`: выделение и освобождение памяти для каждой строки, указатели после `memory.grow`.
  4. `tools/bundle.mjs` и шаблоны `*.txt`: что именно попадает в пакет, нет ли путей сборочной машины.
  5. API `beresta` и `beresta-wasm`: одинаковые имена и поведение (сверьте пару методов).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-29. Ревью: обёртка C#: нативный слой и библиотека

- **Зачем.** P/Invoke, SafeHandle, поиск нативной библиотеки по платформе. Риски — неверный маршалинг (строки, bool, размер перечислений), освобождение хэндла из финализатора в неверном порядке.
- **Приоритет:** желательно до следующего выпуска · ~30–60 минут (файлов: 15, строк: ≈ 871 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-29/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/csharp/Beresta/Native/NativeCalls.cs` (78)
  2. `bindings/csharp/Beresta/Native/NativeHandle.cs` (53)
  3. `bindings/csharp/Beresta/Native/NativeMethods.Document.cs` (40)
  4. `bindings/csharp/Beresta/Native/NativeMethods.Element.cs` (38)
  5. `bindings/csharp/Beresta/Native/NativeMethods.Library.cs` (74)
  6. `bindings/csharp/Beresta/Native/NativeMethods.Node.cs` (71)
  7. `bindings/csharp/Beresta/Native/NativeMethods.Schema.cs` (45)
  8. `bindings/csharp/Beresta/Native/NativeMethods.XPath.cs` (51)
  9. `bindings/csharp/Beresta/Native/NativeResolver.cs` (74)
  10. `bindings/csharp/Beresta/Native/OwnedHandles.cs` (36)
  11. `bindings/csharp/Beresta/Beresta.csproj` (60)
  12. `bindings/csharp/Beresta/BerestaException.cs` (42)
  13. `bindings/csharp/Beresta/BerestaLibrary.cs` (58)
  14. `bindings/csharp/Beresta/Flags.cs` (81)
  15. `bindings/csharp/Beresta/Xml.cs` (70)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `NativeMethods.*`: каждая сигнатура совпадает с `beresta_c.h` (типы, `CallingConvention`, `[MarshalAs]` для строк и bool).
  2. `NativeHandle` / `OwnedHandles`: `ReleaseHandle` не бросает; порядок освобождения узел/документ из финализатора; `DangerousAddRef`.
  3. `NativeResolver`: поиск библиотеки (RID, `runtimes/`), загрузка из текущего каталога — не подменяется ли чужая DLL.
  4. `NativeCalls`: код ошибки → исключение во всех вызовах; сообщения из каталога библиотеки.
  5. `Beresta.csproj`: целевые платформы, упаковка `runtimes/*`.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-30. Ревью: обёртка C#: узлы, схемы, XPath

- **Зачем.** Публичный API C#: XmlDocument/XmlElement/XmlNode, схемы, XPath. Риски — владение узлами при переносе между документами, IDisposable, потоковая проверка.
- **Приоритет:** когда будет время · ~40–70 минут (файлов: 18, строк: ≈ 1047 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-30/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/csharp/Beresta/Nodes/NodeRegistry.cs` (142)
  2. `bindings/csharp/Beresta/Nodes/XmlDocument.Operations.cs` (33)
  3. `bindings/csharp/Beresta/Nodes/XmlDocument.cs` (60)
  4. `bindings/csharp/Beresta/Nodes/XmlElement.Attributes.cs` (45)
  5. `bindings/csharp/Beresta/Nodes/XmlElement.cs` (55)
  6. `bindings/csharp/Beresta/Nodes/XmlNode.Children.cs` (51)
  7. `bindings/csharp/Beresta/Nodes/XmlNode.Content.cs` (45)
  8. `bindings/csharp/Beresta/Nodes/XmlNode.Editing.cs` (49)
  9. `bindings/csharp/Beresta/Nodes/XmlNode.Navigation.cs` (44)
  10. `bindings/csharp/Beresta/Nodes/XmlNode.Ownership.cs` (21)
  11. `bindings/csharp/Beresta/Nodes/XmlNode.Structure.cs` (90)
  12. `bindings/csharp/Beresta/Nodes/XmlNode.cs` (76)
  13. `bindings/csharp/Beresta/Schema/Diagnostic.cs` (56)
  14. `bindings/csharp/Beresta/Schema/XmlSchema.Streaming.cs` (47)
  15. `bindings/csharp/Beresta/Schema/XmlSchema.cs` (64)
  16. `bindings/csharp/Beresta/XPath/XPath.Context.cs` (37)
  17. `bindings/csharp/Beresta/XPath/XPath.Evaluation.cs` (83)
  18. `bindings/csharp/Beresta/XPath/XPath.cs` (49)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `XmlNode.Ownership`: перенос узла в другой документ, узел после `Dispose` документа.
  2. `NodeRegistry`: слабые ссылки, гонка при одновременном доступе из нескольких потоков.
  3. `XmlSchema.Streaming`: освобождение при исключении, повторное использование валидатора.
  4. `XPath.*`: переменные, пространства имён, типы результата.
  5. Соответствие API C# и других обёрток (имена, поведение ошибок).
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-31. Ревью: обёртка Java: нативный слой и библиотека

- **Зачем.** Вызовы Си-интерфейса из Java (Foreign Function & Memory API), загрузка библиотеки из jar, управление памятью. Риски — арены памяти с неверным временем жизни, распаковка библиотеки во временный каталог, неверные layout.
- **Приоритет:** желательно до следующего выпуска · ~50–90 минут (файлов: 20, строк: ≈ 1295 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-31/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/java/src/main/java/org/beresta/internal/DocumentFunctions.java` (37)
  2. `bindings/java/src/main/java/org/beresta/internal/ElementFunctions.java` (38)
  3. `bindings/java/src/main/java/org/beresta/internal/Layouts.java` (39)
  4. `bindings/java/src/main/java/org/beresta/internal/LibraryFunctions.java` (39)
  5. `bindings/java/src/main/java/org/beresta/internal/NativeCalls.java` (87)
  6. `bindings/java/src/main/java/org/beresta/internal/NativeLibrary.java` (36)
  7. `bindings/java/src/main/java/org/beresta/internal/NativeLoader.java` (140)
  8. `bindings/java/src/main/java/org/beresta/internal/NativeMemory.java` (61)
  9. `bindings/java/src/main/java/org/beresta/internal/NativeRelease.java` (51)
  10. `bindings/java/src/main/java/org/beresta/internal/NativeStrings.java` (71)
  11. `bindings/java/src/main/java/org/beresta/internal/NodeCalls.java` (85)
  12. `bindings/java/src/main/java/org/beresta/internal/NodeFunctions.java` (58)
  13. `bindings/java/src/main/java/org/beresta/internal/SchemaFunctions.java` (44)
  14. `bindings/java/src/main/java/org/beresta/internal/XPathFunctions.java` (48)
  15. `bindings/java/src/main/java/org/beresta/Beresta.java` (29)
  16. `bindings/java/src/main/java/org/beresta/BerestaException.java` (63)
  17. `bindings/java/src/main/java/org/beresta/Languages.java` (49)
  18. `bindings/java/src/main/java/org/beresta/Diagnostic.java` (12)
  19. `bindings/java/src/main/java/org/beresta/DiagnosticReader.java` (51)
  20. `bindings/java/pom.xml` (257)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `Layouts` / `*Functions`: каждый дескриптор функции совпадает с `beresta_c.h`.
  2. `NativeMemory` / `NativeStrings`: время жизни арен (строка, возвращённая Си-функцией, после закрытия арены).
  3. `NativeLoader` / `NativeLibrary`: распаковка из jar — права на файл, гонка двух процессов, подмена файла во временном каталоге.
  4. `NativeRelease`: Cleaner, двойное освобождение, порядок узел/документ.
  5. `pom.xml`: подпись, исходники, javadoc, версии плагинов.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-32. Ревью: обёртка Java: узлы, схемы, XPath

- **Зачем.** Публичный API Java. Риски — идентичность обёрток, освобождение документа при живых узлах, потокобезопасность реестра узлов.
- **Приоритет:** когда будет время · ~60–100 минут (файлов: 25, строк: ≈ 1490 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-32/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/java/src/main/java/org/beresta/DocumentOperations.java` (67)
  2. `bindings/java/src/main/java/org/beresta/ElementAttributes.java` (63)
  3. `bindings/java/src/main/java/org/beresta/NodeBase.java` (31)
  4. `bindings/java/src/main/java/org/beresta/NodeChildren.java` (48)
  5. `bindings/java/src/main/java/org/beresta/NodeContent.java` (52)
  6. `bindings/java/src/main/java/org/beresta/NodeEditing.java` (54)
  7. `bindings/java/src/main/java/org/beresta/NodeFactory.java` (63)
  8. `bindings/java/src/main/java/org/beresta/NodeNavigation.java` (39)
  9. `bindings/java/src/main/java/org/beresta/NodeOutput.java` (24)
  10. `bindings/java/src/main/java/org/beresta/NodeRegistry.java` (143)
  11. `bindings/java/src/main/java/org/beresta/NodeSets.java` (88)
  12. `bindings/java/src/main/java/org/beresta/NodeStructure.java` (83)
  13. `bindings/java/src/main/java/org/beresta/NodeType.java` (28)
  14. `bindings/java/src/main/java/org/beresta/ParseFlags.java` (26)
  15. `bindings/java/src/main/java/org/beresta/SchemaValidation.java` (97)
  16. `bindings/java/src/main/java/org/beresta/WriteFlags.java` (16)
  17. `bindings/java/src/main/java/org/beresta/XPath.java` (61)
  18. `bindings/java/src/main/java/org/beresta/XPathBase.java` (16)
  19. `bindings/java/src/main/java/org/beresta/XPathEvaluation.java` (69)
  20. `bindings/java/src/main/java/org/beresta/XPathVariables.java` (49)
  21. `bindings/java/src/main/java/org/beresta/Xml.java` (76)
  22. `bindings/java/src/main/java/org/beresta/XmlDocument.java` (87)
  23. `bindings/java/src/main/java/org/beresta/XmlElement.java` (59)
  24. `bindings/java/src/main/java/org/beresta/XmlNode.java` (68)
  25. `bindings/java/src/main/java/org/beresta/XmlSchema.java` (83)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `NodeRegistry` / `NodeFactory`: одна обёртка на нативный узел, слабые ссылки, потоки.
  2. `NodeEditing` / `NodeStructure`: перенос узла между документами, узел после `close()`.
  3. `SchemaValidation`: освобождение при исключении.
  4. `XPathVariables` / `XPathEvaluation`: типы переменных, пространства имён.
  5. `ParseFlags` / `WriteFlags`: значения совпадают с `beresta_c.h`.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-33. Ревью: обёртка Python

- **Зачем.** ctypes поверх Си-интерфейса: сигнатуры, загрузка библиотеки, обёртки узлов. Риски — неверные `argtypes/restype` (обрезание указателя до int на 64 битах), освобождение в `__del__`, поиск библиотеки.
- **Приоритет:** желательно до следующего выпуска · ~60–100 минут (файлов: 25, строк: ≈ 1436 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-33/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/python/beresta/__init__.py` (42)
  2. `bindings/python/beresta/_diagnostics.py` (45)
  3. `bindings/python/beresta/_document_content.py` (45)
  4. `bindings/python/beresta/_element_attributes.py` (49)
  5. `bindings/python/beresta/_factory.py` (53)
  6. `bindings/python/beresta/_loader.py` (110)
  7. `bindings/python/beresta/_native.py` (42)
  8. `bindings/python/beresta/_node_children.py` (44)
  9. `bindings/python/beresta/_node_content.py` (46)
  10. `bindings/python/beresta/_node_editing.py` (32)
  11. `bindings/python/beresta/_node_navigation.py` (32)
  12. `bindings/python/beresta/_node_structure.py` (50)
  13. `bindings/python/beresta/_schema_streaming.py` (57)
  14. `bindings/python/beresta/_signatures.py` (125)
  15. `bindings/python/beresta/_wrapping.py` (116)
  16. `bindings/python/beresta/_xpath_scalars.py` (52)
  17. `bindings/python/beresta/document.py` (49)
  18. `bindings/python/beresta/element.py` (29)
  19. `bindings/python/beresta/errors.py` (75)
  20. `bindings/python/beresta/flags.py` (50)
  21. `bindings/python/beresta/languages.py` (49)
  22. `bindings/python/beresta/node.py` (70)
  23. `bindings/python/beresta/schema.py` (64)
  24. `bindings/python/beresta/xpath.py` (71)
  25. `bindings/python/pyproject.toml` (39)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `_signatures.py`: у каждой функции заданы `argtypes` и `restype` (без `restype` указатель обрезается до `int`).
  2. `_loader.py` / `_native.py`: порядок поиска библиотеки, загрузка из текущего каталога, `LoadLibraryEx` на Windows.
  3. `_wrapping.py`: освобождение в `__del__` при завершении интерпретатора, узел после закрытия документа.
  4. Строки: `bytes`/`str`, кодировка UTF-8, строки с `\0` внутри.
  5. `pyproject.toml`: что входит в колесо (`_native/` для всех платформ), метаданные.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-34. Ревью: обёртка PHP: нативный слой, библиотека, ошибки, разбор

- **Зачем.** PHP FFI поверх Си-интерфейса: заголовок для FFI, поиск библиотеки, preload. Риски — `beresta.h` для FFI расходится с `beresta_c.h`, освобождение FFI-указателей, загрузка библиотеки по пути из окружения.
- **Приоритет:** желательно до следующего выпуска · ~60–90 минут (файлов: 21, строк: ≈ 1377 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-34/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/php/src/Native/Calls.php` (68)
  2. `bindings/php/src/Native/Handles.php` (75)
  3. `bindings/php/src/Native/Library.php` (136)
  4. `bindings/php/src/Native/LibraryLocator.php` (92)
  5. `bindings/php/src/Native/Platform.php` (90)
  6. `bindings/php/src/Native/Preloader.php` (62)
  7. `bindings/php/src/Native/Strings.php` (64)
  8. `bindings/php/src/Library/Beresta.php` (57)
  9. `bindings/php/src/Library/Internal/Message.php` (30)
  10. `bindings/php/src/Library/Language.php` (67)
  11. `bindings/php/src/Errors/BerestaException.php` (82)
  12. `bindings/php/src/Errors/BerestaThrowable.php` (16)
  13. `bindings/php/src/Errors/ClosedException.php` (31)
  14. `bindings/php/src/Errors/LibraryException.php` (77)
  15. `bindings/php/src/Errors/Status.php` (41)
  16. `bindings/php/src/Parsing/ParseFlags.php` (63)
  17. `bindings/php/src/Parsing/Parser.php` (83)
  18. `bindings/php/src/Parsing/WriteFlags.php` (31)
  19. `bindings/php/resources/beresta.h` (125)
  20. `bindings/php/composer.json` (41)
  21. `bindings/php/autoload.php` (46)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `resources/beresta.h`: сверить с `include/capi/beresta_c.h` (это отдельная копия для FFI).
  2. `Native/LibraryLocator` / `Platform` / `Preloader`: откуда берётся библиотека, переменные окружения, opcache.preload.
  3. `Native/Handles` / `Strings`: освобождение FFI CData, копирование строк, `FFI::free` на пути ошибки.
  4. `Errors/*`: коды Си-интерфейса → исключения, `Status`.
  5. `Parsing/*`: флаги совпадают с `beresta_c.h`; `composer.json`, `autoload.php`.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 

## Т-35. Ревью: обёртка PHP: DOM, схемы, XPath

- **Зачем.** Публичный API PHP: узлы, ArrayAccess для атрибутов, реестр обёрток, перенос между документами. Риски — идентичность обёрток, узел после закрытия документа.
- **Приоритет:** когда будет время · ~70–120 минут (файлов: 26, строк: ≈ 1785 вместе с двуязычными комментариями).
- **Подготовка.** Код в `main` на 2026-11-21. Копии всех файлов — в `tests/manual/review/T-35/` (соответствие копий оригиналам — в `README.md` там же). Файлы по порядку (в скобках — строк):
  1. `bindings/php/src/Dom/Document.php` (107)
  2. `bindings/php/src/Dom/Element.php` (50)
  3. `bindings/php/src/Dom/Internal/DocumentContent.php` (78)
  4. `bindings/php/src/Dom/Internal/DocumentInternals.php` (44)
  5. `bindings/php/src/Dom/Internal/ElementArrayAccess.php` (66)
  6. `bindings/php/src/Dom/Internal/ElementAttributes.php` (70)
  7. `bindings/php/src/Dom/Internal/NodeChildren.php` (62)
  8. `bindings/php/src/Dom/Internal/NodeContent.php` (70)
  9. `bindings/php/src/Dom/Internal/NodeEditing.php` (63)
  10. `bindings/php/src/Dom/Internal/NodeFactory.php` (98)
  11. `bindings/php/src/Dom/Internal/NodeInternals.php` (63)
  12. `bindings/php/src/Dom/Internal/NodeNavigation.php` (56)
  13. `bindings/php/src/Dom/Internal/NodeQueries.php` (35)
  14. `bindings/php/src/Dom/Internal/NodeStructure.php` (68)
  15. `bindings/php/src/Dom/Internal/WrapperRegistry.php` (102)
  16. `bindings/php/src/Dom/Internal/WrapperTransfer.php` (77)
  17. `bindings/php/src/Dom/Node.php` (117)
  18. `bindings/php/src/Dom/NodeType.php` (20)
  19. `bindings/php/src/Schema/Diagnostic.php` (39)
  20. `bindings/php/src/Schema/Internal/DiagnosticsReader.php` (54)
  21. `bindings/php/src/Schema/Internal/SchemaValidation.php` (75)
  22. `bindings/php/src/Schema/Schema.php` (79)
  23. `bindings/php/src/XPath/Internal/NodeSetReader.php` (84)
  24. `bindings/php/src/XPath/Internal/XPathBindings.php` (39)
  25. `bindings/php/src/XPath/Internal/XPathEvaluation.php` (74)
  26. `bindings/php/src/XPath/XPath.php` (95)
- **Шаги — на что смотреть в коде.** Сначала общие пункты О-1…О-8 из `tests/manual/README.md`, затем:
  1. `Dom/Internal/WrapperRegistry` / `WrapperTransfer`: одна обёртка на узел, перенос между документами.
  2. `Dom/Internal/ElementArrayAccess`: `isset`/`unset`/`offsetGet` для несуществующего атрибута.
  3. `Dom/Internal/NodeEditing`: вставка предка в потомка — ошибка Си-интерфейса доходит до PHP-исключения.
  4. `Schema/*`: диагностики, освобождение валидатора.
  5. `XPath/*`: чтение набора узлов (`NodeSetReader`) — длина и освобождение.
- **Что прислать.** Список «`файл:строка` — что не так — какой вход или сценарий это вызовет» (можно коротко). Если всё чисто — так и напишите и какие пункты смотрели. Под каждое найденное я напишу регрессионный тест и исправлю.
- **Результат:** 
