# Берёста — устройство проекта, тесты и расширение

**Русский** · [English](development.en.md)

← [К описанию и преимуществам](../README.md)

## Содержание

1. [Структура проекта](#структура-проекта)
2. [Тесты](#тесты)
3. [Расширение библиотеки](#расширение-библиотеки)
4. [Лицензия](#лицензия)

---

## Структура проекта

```
beresta/
├── CMakeLists.txt  Makefile  README.md  README.en.md  CHANGELOG.md  CHANGELOG.en.md
│   CONTRIBUTING.md  CODE_OF_CONDUCT.md  SECURITY.md (и *.en.md)
├── .github/workflows/            ci.yml (сборка и тесты), release.yml (пакеты, реестры и релиз GitHub)
├── .gitverse/workflows/          ci.yml и release.yml для Gitverse (Linux, релиз без реестров)
├── cmake/                        определение платформы, обёртки, WebAssembly, самопроверка Java
├── include/                      только объявления
│   ├── beresta.hpp               общий заголовок C++
│   ├── capi/                     beresta_c.h — плоский Си-интерфейс; internal/ — барьер исключений
│   ├── core/                     Node, ContainerNode, Element, Document, DocumentType, Text, ...
│   ├── parser/                   XmlParser, ParserOptions, ErrorHandler, ResourceResolver, DomBuilder
│   │   └── internal/             стек входов, бюджет сущностей, пространства имён, декодеры
│   ├── sax/                      SaxParser, ContentHandler, Locator
│   ├── encoding/                 Charset, таблицы, определение кодировки
│   ├── dtd/                      модель, чтение, разбор и проверка DTD
│   ├── content/                  модель содержимого (общая для DTD и XSD), пошаговый курсор
│   ├── xsd/                      Schema, SchemaValidator, ValidatingHandler, типы данных; regex/ — движок шаблонов
│   ├── xpath/                    XPathExpression, XPathValue; internal/ — лексер, парсер, AST
│   ├── traversal/                TreeWalker, NodeIterator, NodeFilter
│   ├── serializer/               XmlSerializer
│   ├── xinclude/                 XInclude
│   ├── io/                       файлы и потоки
│   ├── exception/                DomException, ParseException
│   ├── i18n/                     каталог сообщений: Message, format, setLanguage
│   └── utils/                    символы XML, UTF-8, экранирование, имена, хеш с секретным ключом
├── src/                          реализация, зеркально include/ (≤ 5 функций в файле)
│   ├── i18n/catalogs/            messages_ru.cpp, messages_en.cpp — тексты сообщений
│   └── capi/                     Си-интерфейс: errors, handles, library, document, node, element, xpath, schema
├── bindings/
│   ├── java/                     Maven-проект: org.beresta (API), org.beresta.internal (Panama), selftest
│   ├── csharp/                   Beresta (библиотека, NuGet) и Beresta.SelfTest
│   ├── python/                   пакет beresta, tests/, pyproject.toml
│   ├── node/                     пакет npm: addon/ (Node-API, C++), lib/ (JS + index.d.ts), test/, examples/
│   └── wasm/                     пакет beresta-wasm: lib/ (бэкенд Wasm), tools/ (сборщик), test/, examples/
├── benchmarks/                   замеры против libxml2, Xerces, pugixml, tinyxml2, RapidXML; node/ — против JS-библиотек
├── examples/                     демо на C++ (src/) и на C (capi/)
├── tools/                        release/ — описание и зеркалирование релизов (GitHub → Gitverse); unicode/ — генератор таблиц
└── tests/                        модульные тесты C++ (собственный мини-фреймворк)
    ├── conformance/              прогонщик W3C XML Conformance Test Suite и отчёт
    ├── xsts/                     прогонщик W3C XML Schema Test Suite, список ограничений и отчёт
    └── fuzz/                     цели libFuzzer, словарь, сборщик корпуса
```

Ядро и Си-интерфейс — около **19,8 тысячи строк кода** без комментариев и пустых строк (300 файлов реализации, 97 заголовков; вместе с двуязычными комментариями — около 32 тысяч строк). Обёртки — ещё около 6,8 тысячи строк кода на Java, C#, Python, JavaScript и C++ (аддон Node.js).

### Устройство парсера

```
байты ──► encoding::decodeDocument ──► UTF-8 ──► ParseSession ──► [ValidatingHandler] ──► ContentHandler
              (BOM, декларация,                (итеративный цикл,   (XSD по ходу       ├── DomBuilder ──► Document ──► XInclude
               проверка символов)               стек сущностей,       разбора, если      └── ваш SAX-обработчик
                                                пространства имён,    задана схема)
                                                DTD + валидатор)
```

### Устройство обёрток

```
Java (FFM API) ──┐
C# (P/Invoke) ───┤
Python (ctypes) ─┼──► beresta_c.h ──► барьер исключений ──► ядро C++ (DOM, XPath, XSD, ...)
Node.js (аддон   ┤    (extern "C",     (код + сообщение
  Node-API) ─────┤     void*-дескрипторы) в thread_local)
WebAssembly ─────┤
C, Go, Rust ─────┘
```

Обёртка узла — это нативный дескриптор плюс ссылка на документ-владелец: пока жива обёртка, документ не соберёт сборщик мусора, а после закрытия документа обращение к узлу даёт исключение вместо падения процесса. Каждый документ хранит реестр своих обёрток со слабыми ссылками (паттерн Identity Map): у одного узла одна обёртка. Благодаря реестру перенос (`moveTo`) меняет владельца у всех уже выданных обёрток поддерева сразу.

---

## Тесты

```bash
make test
```

**173 теста C++ в 59 наборах:**
- ядро: копирование, иерархия, элементы, навигация, пространства имён, сериализация, деревья глубиной 100 000 уровней;
- парсер: форматы, содержимое, ошибки, лимиты, SAX (в том числе PI из DTD), кодировки, защита от атак на сложность (повторы атрибутов за порогом перебора, хеш с секретным ключом, линейный рост времени разбора от 25 000 до 200 000 атрибутов), отображение файлов в память (кириллические имена, пустой файл, CRLF и windows-1251);
- DTD: сущности, проверка валидности, внешние ресурсы;
- сериализатор, обход дерева, XPath (пути, оси, функции, ось `namespace`, имена), XSD (типы, схемы, расширенные возможности, `xs:key`/`xs:unique`/`xs:keyref`, пути в сообщениях об ошибках, правила пробелов, группы замены, `xsi:type` с `block`/`final`/`abstract`, `xs:redefine`/`xs:override`; утверждения XSD 1.1 `xs:assert`/`xs:assertion` — пример банковской выписки, отрицательные случаи, точная арифметика копеек, наследование, конструкции XPath 2.0, неподдерживаемый XPath 2.0 как ошибка загрузки, `vc:minVersion`, общая схема из 8 потоков; потоковая проверка: SAX, DOM и готовое дерево дают одинаковые ошибки, потоковые ключи, глубокий документ), рекурсивная схема глубиной 10 000 уровней с `xs:unique` во всех трёх режимах), модель содержимого (пошаговый курсор против общего алгоритма на тысячах случайных моделей), точная десятичная арифметика, XInclude;
- Си-интерфейс: коды ошибок, построение и изменение дерева, перенос узлов между документами (с освобождением исходного документа и откатом отвергнутого переноса), XPath, XML Schema, потоковая проверка (совпадает с проверкой дерева, файлы, ошибки); нагрузка, как у обёрток: 20 потоков на общих схеме, XPath и документе (ошибки у каждого потока свои), 50 000 переносов между документами с проверкой каждого ранее выданного дескриптора;
- языки сообщений: полнота каталогов (у каждого сообщения есть перевод с теми же метками `{N}`), смена языка во время работы, подстановка аргументов;
- регрессии: расхождения с набором W3C (синтаксис, сущности, валидность, кодировки) и находки фаззинга.

Соответствие стандарту и устойчивость к враждебному вводу проверяются отдельно:

```bash
cd tests/conformance && make fetch && make run      # W3C XML Conformance Test Suite -> conformance_report.md
cd tests/xsts && make fetch && make run             # W3C XML Schema Test Suite -> xsts_report.md
cd tests/fuzz && make -j && make corpus && make -j3 fuzz FUZZ_TIME=600 FUZZ_JOBS=3   # libFuzzer, только Clang
```

Тесты проходят в обычной сборке, под AddressSanitizer + UBSan + LeakSanitizer с `-D_GLIBCXX_ASSERTIONS` и под ThreadSanitizer.

Самопроверки обёрток работают без тестовых фреймворков (JUnit, xUnit и pytest не нужны):

```bash
make JAVA=1 CSHARP=1 PYTHON=1 NODE=1 bindings-test
```

Проверяются разбор, навигация, построение, копирование и перенос между документами (в том числе «перенести из A в B, закрыть A, пользоваться узлом в B»), ошибки, запись в другой кодировке, XPath, XML Schema, потоковая проверка (совпадает с деревом, файлы с кириллическими именами), нагрузка (50 000 переносов с сохранением реестра обёрток, 8 потоков на общих схеме и XPath) и языки сообщений: Java — 17 проверок, C# — 16, Python — 21, Node.js — 19 (встроенный `node --test`), WebAssembly — 6 (`make wasm-test`). В CMake всё это (и тесты C++) запускает `ctest`, если включены `BERESTA_BUILD_TESTS` и флаги обёрток.

Где проверено (без единого предупреждения компилятора):

| Платформа | Компиляторы | Что проверено |
|---|---|---|
| Windows 11 x64 | MSVC (Visual Studio 2026), MinGW GCC 15 | CMake и Makefile, тесты C++, `ctest`, самопроверки Java 25 / .NET 9 / Python 3.12 / Node.js 24, `dotnet pack` |
| Ubuntu 26.04 x64 | GCC 15, Clang 21 | CMake и Makefile, тесты C++, `ctest`, самопроверки Java 25 / .NET 10 / Python 3.14 / Node.js 22, `mvn package` (запуск с бинарником из jar), `dotnet pack`, wheel в чистом venv |
| Ubuntu 26.04 x64 | GCC 15 + ASan, UBSan, LeakSanitizer | тесты C++, демо на C и C++: ни утечек памяти, ни неопределённого поведения |
| Ubuntu 26.04 x64 | GCC 15 + ThreadSanitizer | тесты C++, включая 20 потоков на общих дескрипторах Си-интерфейса: ни одной гонки |
| Ubuntu 26.04 x64 | Clang 21 + ASan, UBSan, LeakSanitizer, libFuzzer | тесты C++, прогон W3C XML Conformance Test Suite (100 %), фаззинг трёх целей |
| WebAssembly | Emscripten 3.1.69 | `make wasm` и `emcmake cmake` + `ctest`, самопроверка в Node.js 22, страница-пример в Chromium |

macOS (универсальный бинарник), Linux arm64 и Windows arm64 собираются и тестируются в CI GitHub Actions (`.github/workflows/ci.yml`); на собственном оборудовании автора эти платформы не проверялись.

Как добавить набор C++:
1. Функции тестов в `tests/src/<модуль>/..._tests.cpp`.
2. `registerXxxTests(test::TestRunner&)` в том же файле и объявление в `tests/include/suites/test_suites.hpp`.
3. Строка в таблице `SUITES` в `tests/src/main.cpp`.

Проверки: `TEST_ASSERT(условие)`, `TEST_ASSERT_EQ(строка, "ожидание")`, `TEST_ASSERT_THROWS(инструкция, ТипИсключения)`.

---

## Расширение библиотеки

«Берёста» не использует сторонних библиотек — ни в ядре, ни в обёртках, ни в тестах. Расширяя её, сохраняйте это свойство: алгоритм, таблицу или разборщик лучше написать самостоятельно.

**Новый тип узла:**
1. Значение в `NodeType` (и в `beresta_node_kind` Си-интерфейса).
2. Наследник `CharacterNode` или `ContainerNode` с `getType`, `clone`, `serialize`, `isEqualNode` (и `validateChild` для контейнера).
3. Обработка в `Document::validateChild`, `DomBuilder`, `showMaskFor` и, при необходимости, в осях XPath.

**Новая кодировка:** таблица из 128 кодов в `src/encoding/tables/`, объявление в `single_byte_tables.hpp`, экземпляр и псевдонимы в `charset_registry.cpp`.

**Новая функция XPath:** реализация с сигнатурой `XPathValue f(const FunctionCall&)` и строка в таблице `FUNCTIONS` (`xpath_function_table.cpp`).

**Свой загрузчик ресурсов:** наследник `ResourceResolver`.

**Новое сообщение об ошибке:** константа в `include/i18n/message.hpp` и текст в каждом каталоге `src/i18n/catalogs/` с одинаковыми метками `{N}` (это проверяет тест «полнота каталогов»). В коде — `i18n::format(i18n::Message::ИМЯ, {аргументы})` вместо строкового литерала.

**Новый язык сообщений:** см. [Как добавить язык](guide.md#как-добавить-язык).

**Комментарии** пишутся на двух языках: сначала русский текст, затем его английский перевод в том же комментарии (`// русский` и следующей строкой `// English`, или `// русский / English` для короткого хвостового комментария).

**Новая функция Си-интерфейса:**
1. Объявление с `BERESTA_API` в `include/capi/beresta_c.h` (типы — только C: `void*`, `int`, `int64_t`, `uint32_t`, `double`, `const char*`).
2. Реализация в `src/capi/<группа>/` в обрамлении `BERESTA_CAPI_TRY` … `BERESTA_CAPI_CATCH(...)`: ни одно исключение не должно выйти наружу.
3. Строка-сигнатура в каждой обёртке: `_signatures.py` (Python), `*Functions.java` (Java), `NativeMethods.*.cs` (C#), строка в `bindings/node/addon/src/module/binding_table.cpp` (Node.js; переходник для новой сигнатуры — в `adapters/`), строка в `BINDINGS` в `bindings/wasm/lib/wasm_signatures.js` и имя в `bindings/wasm/exports.json` (WebAssembly; новая сигнатура описывается там же данными). Затем метод в объектном API обёртки.

---

## Лицензия

«Берёста» распространяется под лицензией **MIT** — текст в файле [LICENSE](../LICENSE).

Коротко: библиотеку можно бесплатно использовать, изменять и распространять, в том числе в закрытых и коммерческих продуктах. Единственное условие — сохранять уведомление об авторских правах и текст лицензии в копиях программы. Гарантий нет: программа предоставляется «как есть».

Лицензия уже вложена во все пакеты, поэтому при распространении ничего добавлять вручную не нужно:

| Пакет | Где лежит лицензия |
|---|---|
| jar (Maven) | `META-INF/LICENSE` и элемент `<licenses>` в `pom.xml` |
| NuGet | `LICENSE` в корне пакета и `PackageLicenseExpression` = `MIT` |
| wheel (Python) | `*.dist-info/licenses/LICENSE` и поле `license = "MIT"` в метаданных |
| npm (`beresta`, `beresta-wasm`) | `LICENSE` в корне пакета и поле `"license": "MIT"` в `package.json` |

У `bindings/python`, `bindings/node` и `bindings/wasm` свои копии `LICENSE`, потому что setuptools и npm не берут файлы за пределами каталога пакета. Если меняете корневой `LICENSE`, обновите и их.

Сторонний код в проект не входит: ядро, обёртки и тесты написаны с нуля, поэтому других лицензий и уведомлений (NOTICE, THIRD-PARTY) нет.
