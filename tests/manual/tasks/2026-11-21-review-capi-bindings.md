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
- **Результат:** capi_schema.cpp:13: при передаче out == nullptr в beresta_schema_load выбрасывается исключение с сообщением об ошибке буфера (ARGUMENT_BAD_BUFFER, "data/size/out"), тогда как beresta_schema_load_file и beresta_document_parse сообщают ARGUMENT_NULL с параметром "out". Воспроизводится вызовом beresta_schema_load(xsd_bytes, xsd_size, nullptr) и проверкой beresta_last_error_message().
capi_document.cpp:36, capi_document.cpp:69, capi_document_io.cpp:34, capi_document_io.cpp:58, capi_xpath.cpp:25, capi_xpath_eval.cpp:28, capi_schema.cpp:13, capi_schema.cpp:34, capi_schema.cpp:55, capi_schema_stream.cpp:74, capi_schema_stream.cpp:94: перечисленные функции не обнуляют *out до начала операции, поэтому при ошибке значение выходного указателя остаётся прежним. Например, beresta_document_parse("<broken>", 8, 0, nullptr, &doc) возвращает ошибку разбора, сохраняя прежнее значение doc.
capi_element.cpp:30: в beresta_element_set_attribute строка value копируется в std::string до проверки name на nullptr, из-за чего при передаче name == nullptr выполняется лишнее копирование перед выбросом InvalidArgumentException. Воспроизводится вызовом beresta_element_set_attribute(element, nullptr, "some_value").
capi_handles.cpp:21: функция toNode выполняет безусловный static_cast<Node*>(handle) до dynamic_cast, поэтому передача в любую функцию beresta_node_* валидного дескриптора другого типа (например, beresta_schema или beresta_xpath) вызывает неопределённое поведение и чтение чужой таблицы виртуальных методов вместо возврата BERESTA_INVALID_ARGUMENT, заявленного в beresta_c.h:154. Воспроизводится вызовом beresta_node_type(schema_handle).
capi_options.cpp:20: std::filesystem::u8path устарел начиная с C++20. Если компилятор предупреждает об этом, сборка с -Werror или /WX завершится ошибкой.

- **Разбор.** Загрузка схемы диагностирует пустой `out` как отсутствующий аргумент, функции создания и загрузки зануляют выходной указатель до операции. Имя атрибута проверяется до копирования значения, зарегистрированный дескриптор другого типа отклоняется до обращения к узлу. Для UTF-8 путей C++20 применяется перегрузка с `std::u8string`, для C++17 используется `u8path`. Модульные тесты Си-интерфейса проверяют код ошибки для дескриптора другого типа и состояние выходного указателя после отказа.

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
- **Результат:** В call_frame_values.cpp:60 и call_frame_values.cpp:71 методы CallFrame::integer и CallFrame::flags проверяют только ValueType::NUMBER и вызывают napi_get_value_int64 и napi_get_value_uint32, которые молча усекают дробную часть, превращают NaN и Infinity в 0, а отрицательные числа в uint32 переводят в гигантские положительные маски. Воспроизводится вызовом node_child(doc, 1.5) или document_parse(buf, -1), где вместо ошибки типа передадутся индекс 1 и маска 0xFFFFFFFF.
В addon_state.cpp:54 в функции initialize не проверяется тип переданного аргумента и результат napi_create_reference. Если вызвать из JavaScript init(null) или init(123), createReference вернёт ошибку, state->errorClass молча останется nullptr, а аддон не выбросит TypeError и при последующих ошибках начнёт бросать стандартный Error вместо BerestaError.
В results_strings.cpp:13 в createString игнорируется возвращаемый статус napi_create_string_utf8, а строка char* сразу освобождается в makeOwnedString через beresta_string_free. Если создать строку не удалось, адаптер возвращает result без обработки кода ошибки. Воспроизводится сериализацией документа размером больше максимальной длины строки V8 через node_to_string(doc, 0).
В module.cpp:47 в цикле registerAll не проверяется результат exportFunction. Если регистрация одной из функций упадёт с ошибкой и оставит в env активное исключение, цикл продолжит вызывать napi_create_function и napi_set_named_property при наличии pending exception в env.
В adapters_text.cpp:25 в adaptBorrowedOfInt прочитанный std::int64_t приводится к int через static_cast<int>(value) без проверки диапазона. Вызов status_name(4294967296) или message_text(4294967296) обрежет число до 0 (BERESTA_OK) вместо передачи невалидного кода в C функцию

- **Разбор.** Аддон проверяет границы целых аргументов и масок флагов, а также тип конструктора ошибки. Предыдущая ссылка сохраняется при отказе регистрации. Ошибки создания строки и экспорта функций обрабатываются в месте вызова. Проверены граничные аргументы и сохранение класса ошибок.

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
- **Результат:** node_structure.js:18 (remove()) и registry.js:53 (store()): при вызове node.remove() запись о дескрипторе остаётся в document[REGISTRY], поэтому если после const a = doc.root.appendElement('a'); a.remove(); создать новый элемент const b = doc.root.appendElement('b') и C++ аллокатор переиспользует освобождённый дескриптор, wrap() вернёт старую обёртку a вместо новой (b === a).
node_children.js:22 (child(index)): при передаче NaN в node.child(NaN) значение Math.trunc(NaN) остаётся NaN и передаётся в native.node_child, где N-API преобразует NaN в 0 и молча вернёт первого ребёнка вместо ошибки или null.
factory.js:27 (parse) и factory.js:35 (parseFile): при вызове parse('<a/>', null) параметр по умолчанию options = {} не срабатывает, и код падает с необработанным TypeError: Cannot read properties of null при обращении к options.flags.
native.js:48 (loadNative): вызов require(file) не обёрнут в try/catch, поэтому при наличии битого или несовместимого по ABI файла beresta.node на диске модуль выбросит системную ошибку Node.js (ERR_DLOPEN_FAILED) вместо задокументированного BerestaLoadError.
index.d.ts:119 (copyTo<T extends XmlNode>(parent: XmlNode): T): возвращаемый тип задан через несвязанный generic параметр T, из за чего без явного указания параметра типа element.copyTo(parent) выводится как XmlNode вместо XmlElement, а при ручном указании textNode.copyTo<XmlElement>(parent) компилятор без ошибки типизирует текстовый узел как элемент.
index.d.ts:183 (XPathOptions.variables): в типе опций конструктора указано Record<string, string | number> без boolean, тогда как метод XPath.set на строке 192 принимает string | number | boolean

- **Разбор.** После успешного удаления из реестра исключаются узел и его потомки. Живые обёртки отклоняют обращение к удалённому дескриптору. Исправлены обработка NaN и null в опциях разбора, диагностика загрузки аддона и типы TypeScript для copyTo и переменных XPath.

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
- **Результат:** bindings/wasm/tools/bundle_runtime.mjs.txt:46 при первом вызове init(options) сохраняет промис createApi(options) в переменную ready и не сбрасывает её при отклонении промиса. Если загрузка или компиляция .wasm завершится ошибкой (например, передан неверный путь в locateFile или временно недоступна сеть), все последующие вызовы init() в том же процессе будут возвращать этот же отклонённый промис без повторной попытки инициализации. Воспроизводится вызовом await init({ wasmBinary: new Uint8Array([0]) }).catch(() => {}) и последующим await init({ wasmBinary: validBytes }).
bindings/wasm/lib/wasm_call.js:56 при конвертации аргумента вида index выполняет BigInt(Math.trunc(value)) без проверки диапазона int64. При передаче 1e25 или Number.MAX_VALUE в node.child(1e25) или nodeset_node(set, 1e25) формируется BigInt, выходящий за пределы знакового 64-битного целого int64_t в C, и при передаче в Wasm происходит молчаливое обрезание битов до 64-битного значения вместо ошибки аргумента.
bindings/wasm/lib/wasm_call.js:27 при конвертации аргумента вида handle выполняет Number(value) без проверки, что BigInt помещается в беззнаковый 32-битный указатель wasm32 (value >= 0n && value <= 0xFFFFFFFFn). Если передать в обёртку BigInt со значением больше 4294967295n (например, 0x100000004n), в Wasm будут переданы младшие 32 бита (4) вместо выброса TypeError с кодом INVALID_ARGUMENT.
bindings/wasm/tools/bundle.mjs:36 ищет зависимости регулярным выражением const REQUIRE = /require\('([^']+)'\)/g; по сырому тексту JS-файла без учёта комментариев и строковых литералов. Любой комментарий или строковая константа в bindings/node/lib, содержащая подстроку вида require('./что-угодно'), заставит сборщик упасть с ENOENT на fs.readFileSync либо подменит текст внутри строки кода на require(номер)

- **Разбор.** После отказа init разрешает повторную попытку. Конвертер проверяет диапазоны int64 и wasm32. Сборщик пропускает комментарии и строковые литералы. Бандл собран, сканер зависимостей, повторная инициализация и числовые границы проверены отдельно. Интеграционный тест с ядром остаётся непроверенным: локальный бинарник лишён экспорта beresta_schema_validate_bytes, Emscripten недоступен.

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
- **Результат:** Xml.cs:49-53 — в методе SchemaPointer указатель из SafeHandle извлекается через DangerousGetHandle() без захвата счётчика ссылок DangerousAddRef и DangerousRelease, из-за чего GC.KeepAlive(schema) в Xml.Parse и Xml.ParseFile спасает только управляемую обёртку от сборки мусора, но не блокирует освобождение нативного ресурса при параллельном вызове Dispose(). При одновременном вызове schema.Dispose() из одного потока и Xml.Parse(xml, schema: schema) из другого гонка (TOCTOU) может привести к чтению освобождённой нативной памяти (Use-After-Free) внутри C-библиотеки.
Xml.cs:26 — метод Xml.Parse(byte[] data, ...) обращается к свойству data.LongLength без проверки data на null. Воспроизводится вызовом Xml.Parse((byte[])null!), что приводит к необработанному NullReferenceException вместо ожидаемого ArgumentNullException

- **Разбор.** Parse и ParseFile удерживают SafeHandle схемы до возврата из нативного вызова. Для null вместо массива байтов Parse бросает ArgumentNullException. Сборка netstandard2.1 подтвердила доступность Marshal.PtrToStringUTF8.

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
- **Результат:** NodeRegistry.cs:109 (Transfer) Проверка deep = wrappers.Count > 1 пропускает обход потомков, если единственная запись в реестре source относится к потомку переносимого узла.
XmlNode.cs:60 (Equals) Сравнение node.pointer == pointer не проверяет, что оба узла принадлежат одному документу (ReferenceEquals(Document, node.Document)). Если вызвать doc1.Dispose(), а затем создать doc2 и распределитель памяти переиспользует освобождённый адрес, oldNodeFromDoc1.Equals(newNodeFromDoc2) вернёт true для совершенно разных узлов из разных документов.
XmlNode.Structure.cs:87 (Wrap) Между проверкой Find(pointer, element) (строка 87) и записью Store(pointer, node) (строка 93) нет единой блокировки реестра. При одновременном чтении одного узла из двух потоков оба потока получат null из Find, создадут два разных экземпляра XmlNode/XmlElement и перезапишут друг друга в словаре, нарушая инвариант Identity Map (ReferenceEquals(a, b) == false при a.Equals(b) == true)

- **Разбор.** Реестр создаёт обёртку под одной блокировкой и при переносе учитывает, какому узлу принадлежит единственная запись. Equals учитывает документ-владелец, хеш остаётся стабильным при переносе. Проверена идентичность обёрток при параллельном чтении.

Рабочие копии Т-26–Т-30 сохранены в `tests/manual/review/`.

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
- **Результат:** NativeStrings.java:27 borrowed вызывает ((MemorySegment) result).reinterpret(Long.MAX_VALUE).getString(0) без проверки address() == 0. Вызов NativeStrings.message(-1) получает NULL от beresta_message_text и пытается прочитать строку по нулевому адресу.
NativeStrings.java:37 message поочерёдно делает text.replace("{" + index + "}", String.valueOf(arguments[index])). Если в первом аргументе из XML-документа попадётся текст "{1}", следующая итерация подставит туда второй аргумент и исказит сообщение. Воспроизводится вызовом NativeStrings.message(code, "a{1}b", "X").
BerestaException.java:24 конструктор дважды вызывает нативную функцию через nameOf(code): первый раз в super("[" + nameOf(code) + "] " + detail) и второй раз в this.codeName = nameOf(code). При создании любого BerestaException делается лишний нативный вызов.

## Разбор

Исправлены обработка NULL в NativeStrings, подстановка аргументов по исходному шаблону и повторное получение имени статуса в BerestaException. Замечание о правах каталога не подтвердилось для OpenJDK: TempFileHelper задаёт владельцу rwx------ при создании временного каталога на POSIX.

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
- **Результат:** XmlSchema.java:63 отсутствует проверка закрытия схемы при вызове handle(), в отличие от XmlDocument и XPath. Вызов schema.close(), а затем schema.validate(doc) или Xml.parse(xml, 0, schema) передаст освобождённый указатель в C++.
NodeFactory.java:39 проверка find() и запись store() не атомарны, а NodeRegistry.java:65 безусловно перезаписывает существующую запись в словаре. При обращении к одному узлу из двух потоков оба могут создать по обёртке. Если вторая перезапишет первую в реестре, первая при moveTo() сохранит старый owner, а после oldDoc.close() её handle() выбросит IllegalStateException, хотя перенесённый узел жив.
NodeRegistry.java:108 условие deep = wrappers.size() > 1 опирается на число записей в словаре. Если обёртка moved отсутствует в реестре, а единственной записью осталась обёртка её потомка, при moved.moveTo(otherDoc) флаг deep станет false и потомок сохранит прежний owner. После закрытия исходного документа его handle() выбросит IllegalStateException.
XmlNode.java:51 метод equals() сравнивает только числовые адреса указателей без проверки активности документов. Если закрыть doc1, создать doc2 и аллокатор переиспользует адрес освобождённого узла для нового, вызов closedNodeFromDoc1.equals(newNodeFromDoc2) вернёт true.
XmlElement.java:37 и NodeSets.java:46 возвращаемое значение ATTRIBUTE_COUNT и NODESET_SIZE не проверяется на отрицательный код ошибки, хотя в NodeChildren.java:17 для CHILD_COUNT такая проверка есть. При нативной ошибке цикл по атрибутам или узлам молча пропустится и вернёт пустую коллекцию вместо BerestaException

## Разбор

Все пять замечаний учтены. Закрытая XmlSchema отклоняет запрос дескриптора, в том числе при разборе XML. Поиск и создание обёртки выполняются под одной блокировкой. Перенос учитывает потомков при отсутствии корня в реестре. equals учитывает закрытие документа и сохраняет рефлексивность. Ошибки получения размеров коллекций передаются вызывающему коду.

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
- **Результат:** _node_structure.py:21 в remove() освобождает нативный узел в C++, но оставляет живой саму обёртку и её потомков в self._document._wrappers с ненулевым _pointer, поэтому любое последующее обращение к удалённому узлу (например, child = doc.root.first_child; child.remove(); child.name) приводит к обращению к освобождённой памяти (use-after-free).
_native.py:32 в encode() кодирует строку в UTF-8 без проверки на нулевой байт \x00, из-за чего при передаче строки с символом \0 внутрь c_char_p (например, elem.set_attribute("k", "a\x00b")) C-функция молча обрезает строку по первому нулю без исключения.
languages.py:21 и languages.py:22 объявляют коды сообщений XPATH_CLOSED = 2 и SCHEMA_CLOSED = 3, однако в xpath.py:32 и schema.py:35 у классов XPath и Schema отсутствуют методы close(), свойство closed и проверка дескриптора перед вызовом C-функций, поэтому явное освобождение ресурсов до сборки мусора невозможно.
_loader.py:89 в load_library() проверяет существование файла только при os.path.isabs(candidate), поэтому если передать в переменную окружения BERESTA_LIBRARY относительный путь к несуществующему файлу (например, BERESTA_LIBRARY=missing.dll), ctypes.CDLL выбросит системный OSError напрямую вместо формирования понятного сообщения с перечислением проверенных путей в tried.
_wrapping.py:67 в wrap_node() читает document.raw_handle и вызывает LIB.beresta_node_type(pointer) без проверки document.closed, что при передаче закрытого документа приводит к чтению освобождённой памяти в нативном коде вместо выброса RuntimeError

## Разбор

Удаление поддерева инвалидирует выданные обёртки после успешного нативного вызова. wrap_node проверяет документ перед чтением типа. Строковые аргументы C API отвергают встроенный NUL; буферы XML с явной длиной по-прежнему передаются целиком. XPath и Schema поддерживают close() и closed с однократным освобождением через weakref.finalize. Относительный BERESTA_LIBRARY разрешается от рабочего каталога. Сценарий преждевременной выгрузки модулей для финализатора не подтверждён.

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
- **Результат:** Preloader.php:37 при подстановке пути в #define FFI_LIB не экранируются кавычки и переносы строк. Если передать в Beresta::preload() или переменную окружения BERESTA_LIBRARY путь с кавычкой или переводом строки (например, /tmp/my"dir/libberesta.so), препроцессор FFI упадет с синтаксической ошибкой в FFI::load().
Preloader.php:27 имя временного файла beresta_ffi_<md5>.h в sys_get_temp_dir() зависит только от пути и перезаписывается напрямую через file_put_contents() без атомарного переименования. Если Beresta::preload() сначала выполнится от root (например, при проверке конфигурации в CLI), а затем запустится воркер PHP-FPM от www-data, запись в уже созданный файл упадет с ошибкой доступа и выбросит LibraryException::preloadFailed, а при одновременном старте процессов FFI::load() может прочитать недописанный файл.
Language.php:48 цикл последовательного вызова str_replace('{' . $index . '}', $argument, $text) выполняет повторную подстановку по уже вставленным значениям. Если аргумент $arguments[0] содержит подстроку {1} (например, имя узла или путь из XML), на следующей итерации она ошибочно заменится на $arguments[1] вместо использования strtr().

## Разбор

Preloader отклоняет символы, непредставимые в строке FFI_LIB, создаёт отдельный временный заголовок для каждого вызова, проверяет полноту записи и удаляет файл после загрузки. Language::message подставляет аргументы за один проход. Реализация Handles::address сохранена: индекс [0] читает значение по указателю intptr_t*, что согласуется с результатами проверки PHP FFI в журнале проекта.

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
- **Результат:** NodeStructure.php:26 забывает в реестре документа адрес только самого удаляемого узла ($this->document()->wrappers()->forget($address)), оставляя там адреса всех его потомков. Если после удаления поддерева создать новый узел того же типа и аллокатор переиспользует освобождённый адрес памяти, NodeFactory.php:45 найдёт старую живую обёртку в реестре, увидит совпадение kindCode() и вернёт старый PHP объект удалённого потомка вместо нового. Воспроизводится так: создаём $a = $root->appendElement('a') и $b = $a->appendElement('b'), вызываем $a->remove(), затем создаём $c = $root->appendElement('c'), и при совпадении адреса в памяти $c === $b окажется true.
WrapperTransfer.php:30 определяет необходимость обхода потомков условием $deep = $source->wrappers()->count() > 1, исходя из предположения, что в реестре исходного документа обязательно лежит сама переносимая обёртка $moved. Если обёртка живого переносимого узла отсутствует в $source->wrappers(), а в реестре осталась ровно одна обёртка его потомка, count() вернёт 1, $deep станет false, и потомок не сменит владельца document. После $moved->moveTo($target) и $source->close() обращение к потомку выбросит ClosedException, хотя узел находится в открытом целевом документе.

## Разбор

До удаления собираются живые обёртки поддерева; после успеха они исключаются из реестра и запрещают доступ к нативному узлу. При переносе проверяется наличие корня в реестре. Единственный зарегистрированный потомок получает нового владельца и сохраняет идентичность.
