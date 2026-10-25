# Берёста — Си-интерфейс и обёртки для других языков

**Русский** · [English](bindings.en.md)

← [К описанию и преимуществам](../README.md)

## Содержание

1. [Другие языки: Си-интерфейс](#другие-языки-си-интерфейс)
2. [Java (Project Panama)](#java-project-panama)
3. [C# (P/Invoke)](#c-pinvoke)
4. [Python (ctypes)](#python-ctypes)
5. [PHP (FFI)](#php-ffi)
6. [Node.js (Node-API)](#nodejs-node-api)
7. [WebAssembly (браузер и Workers)](#webassembly-браузер-и-workers)

---

## Другие языки: Си-интерфейс

Заголовок `include/capi/beresta_c.h` написан на C99 и подключается и в `.c`, и в `.cpp`. На нём построены все обёртки, и им же можно пользоваться напрямую из C, Go (cgo), Rust (FFI), Delphi и любого языка, который умеет вызывать функции C.

```c
#include "capi/beresta_c.h"

const char* xml = "<shop><item code=\"A1\">Чай</item><item code=\"B2\">Кофе</item></shop>";
beresta_document document = NULL;
beresta_xpath xpath = NULL;

if (beresta_document_parse(xml, (int64_t)strlen(xml), BERESTA_PARSE_DEFAULT, NULL, &document) != BERESTA_OK)
{
    fprintf(stderr, "[%s] %s\n", beresta_status_name(beresta_last_error_code()), beresta_last_error_message());
    return 1;
}
beresta_xpath_compile("//item[@code = $code]", &xpath);
beresta_xpath_set_string(xpath, "code", "B2");

char* name = beresta_xpath_string(xpath, document);   /* "Кофе" */
printf("%s\n", name);
beresta_string_free(name);                             /* строку освобождает библиотека */

beresta_xpath_free(xpath);
beresta_document_free(document);                       /* документ вместе со всеми узлами */
```

Полный пример — `examples/capi/capi_demo.c` (собирается вместе с демо). Подключение к своей программе:

```bash
gcc -std=c99 -Ipath/to/beresta/include app.c -Lpath/to/beresta/bin -lberesta -o app
```

### Соглашения (одинаковы для всех 71 функции)

| Правило | Пояснение |
|---|---|
| Дескрипторы | `beresta_document`, `beresta_node`, `beresta_xpath`, `beresta_nodeset`, `beresta_schema`, `beresta_diagnostics` — это `void*` |
| Статус | функции, возвращающие `int`, отдают `BERESTA_OK` (0) при успехе или код ошибки |
| NULL | функции, возвращающие дескриптор или `char*`, отдают NULL при ошибке. NULL с кодом 0 означает «объекта нет» (например, нет следующего соседа) |
| Последняя ошибка | `beresta_last_error_code()` и `beresta_last_error_message()` — своя у каждого потока, как `errno`. Каждый вызов её сбрасывает |
| Строки | всегда UTF-8. `char*` от библиотеки освобождается через `beresta_string_free`, `const char*` — заимствованная |
| Размеры и индексы | `int64_t` — одинаковый тип во всех языках; `-1` в результате означает ошибку |
| Владение | документ, схему, выражение, набор узлов и список диагностик освобождает вызывающий (`beresta_*_free`). Узел принадлежит документу; `beresta_node_move_to` передаёт его (с потомками) другому документу, дескрипторы при этом не меняются |
| Потоки | разные документы — в разных потоках параллельно. Схему, скомпилированное выражение XPath и документ, который никто не меняет, можно делить между потоками: тест гоняет 20 потоков на общих дескрипторах под ThreadSanitizer |

| Группа | Функции |
|---|---|
| Библиотека | `version`, `last_error_code`, `last_error_message`, `status_name`, `string_free` |
| Языки сообщений | `set_language`, `language`, `languages`, `message_text` |
| Документы | `document_create`, `document_parse` (байты в любой кодировке), `document_parse_file`, `document_clone`, `document_root`, `document_element_by_id`, `document_save` (любая выходная кодировка), `document_free` |
| Узлы | `node_type`, `node_name`, `node_text`, `node_to_string`, `node_parent`, `node_first_child`, `node_last_child`, `node_next_sibling`, `node_previous_sibling`, `node_owner_document`, `node_child_count`, `node_child` |
| Изменение дерева | `node_set_text`, `node_remove`, `node_copy_to` (в том числе между документами), `node_move_to` (перенос без копирования, в том числе между документами), `node_append_element`, `node_append_text`, `node_append_comment`, `node_append_cdata`, `node_append_pi` |
| Атрибуты | `element_get_attribute`, `element_set_attribute`, `element_has_attribute`, `element_remove_attribute`, `element_attribute_count`, `element_attribute_name`, `element_attribute_value`, `element_local_name`, `element_namespace_uri` |
| XPath | `xpath_compile`, `xpath_bind_namespace`, `xpath_set_string`, `xpath_set_number`, `xpath_select`, `xpath_string`, `xpath_number`, `xpath_boolean`, `nodeset_size`, `nodeset_node`, `nodeset_value`, `xpath_free`, `nodeset_free` |
| XML Schema | `schema_load`, `schema_load_file`, `schema_validate` (готовое дерево), `schema_validate_bytes` и `schema_validate_file` (поток без дерева), `diagnostics_count`, `diagnostics_message`, `diagnostics_line`, `diagnostics_column`, `schema_free`, `diagnostics_free` |

Флаги разбора — битовая маска `BERESTA_PARSE_*`: `PRESERVE_WHITESPACE`, `DISCARD_COMMENTS`, `COALESCE_CDATA`, `VALIDATE_DTD`, `NO_NAMESPACES`, `XINCLUDE`, `FILE_RESOURCES`, `TRACK_LOCATIONS`. `FILE_RESOURCES` разрешает внешние DTD, сущности и XInclude, но только из каталога разбираемого файла. `TRACK_LOCATIONS` запоминает строку и столбец каждого узла: тогда `schema_validate` сообщает места нарушений (без флага — 0; разбор с ним примерно на 8 % медленнее).

**Потоковая проверка без дерева.** `beresta_schema_validate_bytes` и `beresta_schema_validate_file` принимают те же аргументы, что `beresta_document_parse` и `beresta_document_parse_file`, но вместо документа возвращают список нарушений. Внутри работает SAX-парсер с `ValidatingHandler`: дерево не строится, память — O(глубины документа) плюс значения ID и таблицы ключей, а файл отображается в память, а не копируется. Нарушения схемы — не ошибка вызова (`BERESTA_OK` и список), синтаксическая ошибка XML — `BERESTA_PARSE_ERROR`. Места нарушений известны всегда, флаг `TRACK_LOCATIONS` не нужен.

```c
beresta_diagnostics problems = NULL;
if (beresta_schema_validate_file("export.xml", BERESTA_PARSE_DEFAULT, schema, &problems) == BERESTA_OK)
{
    for (int64_t i = 0; i < beresta_diagnostics_count(problems); ++i)
    {
        char* text = beresta_diagnostics_message(problems, i);
        printf("%lld: %s\n", (long long)beresta_diagnostics_line(problems, i), text);
        beresta_string_free(text);
    }
    beresta_diagnostics_free(problems);
}
``` Флаги вывода — `BERESTA_WRITE_PRETTY` и `BERESTA_WRITE_DECLARATION`. Маска выбрана вместо структуры настроек сознательно: целое число одинаково передаётся из любого языка.

Коды 1–15 совпадают с `DomErrorCode` + 1 (`BERESTA_PARSE_ERROR`, `BERESTA_VALIDATION_ERROR`...). Ещё три кода есть только в Си-интерфейсе: `BERESTA_INVALID_ARGUMENT` (NULL или дескриптор не того типа), `BERESTA_OUT_OF_MEMORY`, `BERESTA_UNKNOWN_ERROR`.

---

## Java (Project Panama)

Обёртка — Maven-проект `bindings/java` (пакет `org.beresta`, Java 22+). Она использует Foreign Function & Memory API: JNI, заголовков `jni.h` и кода на C для связки нет. Зависимостей тоже нет.

```java
import org.beresta.*;

try (XmlDocument document = Xml.parse("<shop><item code='A1'>Чай</item></shop>"))
{
    XmlElement item = (XmlElement) document.root().firstChild();
    System.out.println(item.getAttribute("code") + " " + item.text());       // A1 Чай

    try (XPath query = new XPath("//item[@code = $c]"))
    {
        System.out.println(query.set("c", "A1").string(document));            // Чай
    }
}

try (XmlDocument order = XmlDocument.create("order"))
{
    XmlElement item = order.root().appendElement("item");
    item.setAttribute("qty", "2");
    item.appendText("Мёд & орехи");
    System.out.println(order.toXml());   // <order><item qty="2">Мёд &amp; орехи</item></order>
    order.save(Path.of("order.xml"), WriteFlags.PRETTY, "windows-1251");
}

try (XmlSchema schema = XmlSchema.fromFile(Path.of("order.xsd"));
     XmlDocument document = Xml.parseFile(Path.of("order.xml"), ParseFlags.TRACK_LOCATIONS, null))
{
    for (Diagnostic problem : schema.validate(document))                    // готовое дерево
        System.out.println(problem.line() + ": " + problem.message());
    List<Diagnostic> export = schema.validateFile(Path.of("export.xml"));   // поток без дерева, хоть гигабайты
}
catch (BerestaException error)
{
    System.out.println(error.codeName() + " " + error.detail());           // PARSE_ERROR строка 1, ...
}
```

Сборка:

```bash
make JAVA=1
```
```bash
mvn -f bindings/java/pom.xml package
```

Запуск приложения:

```bash
java --enable-native-access=ALL-UNNAMED -cp beresta-2.1.4.jar:app.jar com.example.App
```

Как это устроено:
- `NativeLoader` определяет платформу по `os.name` и `os.arch`, извлекает `natives/<os>-<arch>/<файл>` из jar во временный каталог и загружает его через `SymbolLookup.libraryLookup`. Путь можно задать явно: `-Dberesta.library.path=/path/libberesta.so`.
- Дескрипторы вызовов (`MethodHandle`) строятся один раз по `FunctionDescriptor`, строки передаются через `Arena.ofConfined()` и освобождаются сразу после вызова.
- `XmlDocument`, `XPath`, `XmlSchema` реализуют `AutoCloseable`. Если объект забыли закрыть, нативную память освободит `Cleaner`.
- Узел хранит ссылку на свой документ. После `close()` обращение к узлу даёт `IllegalStateException`, а не падение JVM.
- Методы узла разнесены по интерфейсам (`NodeContent`, `NodeNavigation`, `NodeChildren`, `NodeEditing`, `NodeStructure`, `ElementAttributes`), проверка по схеме — в `SchemaValidation`: можно принимать ровно ту часть API, которая нужна.
- Горячие вызовы обхода (навигация, тип узла, ребёнок по индексу, узлы результата XPath) идут через `invokeExact` с точным типом, остальные — через общий `invokeWithArguments`. JIT встраивает `invokeExact` почти как прямой вызов функции C, поэтому обход дерева из Java стал в 1,4–1,7 раза быстрее.

Самопроверка без Maven и JUnit (нужен только JDK 22+): `make JAVA=1 java-test`.

---

## C# (P/Invoke)

Обёртка — проект `bindings/csharp/Beresta` (пространство имён `Beresta`, сборка `Beresta.NET.dll`, целевые платформы `netstandard2.1` и `net9.0`).

```csharp
using Beresta;

using (XmlDocument document = Xml.Parse("<shop><item code='A1'>Чай</item></shop>"))
{
    XmlElement item = (XmlElement)document.Root!.FirstChild!;
    Console.WriteLine(item.GetAttribute("code") + " " + item.Text);            // A1 Чай

    using XPath query = new XPath("//item[@code = $c]").Set("c", "A1");
    Console.WriteLine(query.EvaluateString(document));                        // Чай
}

using (XmlDocument order = Xml.NewDocument("order"))
{
    XmlElement item = order.Root!.AppendElement("item");
    item.SetAttribute("qty", "2");
    item.AppendText("Мёд & орехи");
    order.Save("order.xml", WriteFlags.Pretty, "windows-1251");
}

try
{
    using XmlSchema schema = XmlSchema.FromFile("order.xsd");
    using XmlDocument document = Xml.ParseFile("order.xml", ParseFlags.TrackLocations);
    foreach (Diagnostic problem in schema.Validate(document))               // готовое дерево
        Console.WriteLine(problem.Line + ": " + problem.Message);
    var export = schema.ValidateFile("export.xml");                         // поток без дерева, хоть гигабайты
}
catch (BerestaException error)
{
    Console.WriteLine(error.CodeName + " " + error.Detail);
}
```

Сборка пакета NuGet:

```bash
make CSHARP=1 csharp-package
```

Как это устроено:
- Функции объявлены через `[DllImport]`; входные строки передаются как `UnmanagedType.LPUTF8Str`, выходные `char*` читаются и сразу освобождаются.
- Документ, выражение и схема владеют нативной памятью через `SafeHandle`: освобождение гарантировано и при `Dispose`, и при финализации, а счётчик ссылок не даёт освободить ресурс во время вызова.
- В пакете NuGet бинарники лежат в стандартной раскладке `runtimes/<rid>/native/`, и .NET сам выбирает нужный. При ссылке на проект (`ProjectReference`) их находит собственный резолвер (`NativeLibrary.SetDllImportResolver`). Путь можно задать явно переменной окружения `BERESTA_LIBRARY`.
- Сборка называется `Beresta.NET.dll`, а не `Beresta.dll`: на Windows регистр имён файлов не различается, и управляемая сборка совпала бы с нативной `beresta.dll`.

Самопроверка — консольная программа без xUnit: `make CSHARP=1 csharp-test`. Она собирается под самую новую среду установленного SDK (`net9.0` с SDK 9, `net10.0` с SDK 10), поэтому запускается без доустановки runtime.

---

## Python (ctypes)

Пакет `bindings/python/beresta` (Python 3.8+). Он использует только стандартный модуль `ctypes`: ничего не нужно устанавливать и ничего не компилируется на машине пользователя.

```python
import beresta
from beresta import BerestaError, ParseFlags, Schema, WriteFlags, XPath

with beresta.parse("<shop><item code='A1'>Чай</item></shop>") as document:
    item = document.root.first_child
    print(item.get_attribute("code"), item.text)                      # A1 Чай
    print(XPath("//item[@code = $c]", variables={"c": "A1"}).string(document))
    print([child.name for child in document.root])                    # ['item']

with beresta.new_document("order") as order:
    item = order.root.append_element("item")
    item.set_attribute("qty", "2")
    item.append_text("Мёд & орехи")
    order.save("order.xml", WriteFlags.PRETTY, encoding="windows-1251")

try:
    schema = Schema.from_file("order.xsd")
    with beresta.parse_file("order.xml", ParseFlags.TRACK_LOCATIONS) as document:
        for problem in schema.validate(document):                     # готовое дерево
            print(problem.line, problem.message)
        prices = XPath("//item/@price").select_values(document)      # строки без объектов-узлов
    export = schema.validate_file("export.xml")                       # поток без дерева, хоть гигабайты
except BerestaError as error:
    print(error.name, error.message)
```

Подготовка пакета:

```bash
make PYTHON=1
```
```bash
python -m pip wheel bindings/python -w dist
```

Как это устроено:
- Для **каждой** функции заданы `restype` и `argtypes`. Без них ctypes обрезал бы 64-битные указатели до `int`.
- Возвращаемые `char*` объявлены как `c_void_p`, а не `c_char_p`: так указатель не теряется, и строку можно освободить.
- Документ работает как контекстный менеджер (`with`). Если его не закрыли, память освободит `weakref.finalize`.
- Узлы — обычные объекты Python: `len(node)`, `node[i]`, `for child in node`, свойства `text`, `name`, `parent`.
- Обёртка запоминает тип узла и классы обёрток, поэтому на шаг обхода приходится меньше вызовов ctypes, и пошаговый обход стал в 1,6 раза быстрее. Но быстрее всего — не ходить по дереву из Python вовсе: `select_values` собирает строки внутри ядра.
- Библиотека ищется в `BERESTA_LIBRARY`, затем в `beresta/_native/<os>-<arch>/`, затем в системе.

Самопроверка на стандартном `unittest`: `make PYTHON=1 python-test`. Makefile запускает `python` на Windows и `python3` на Linux и macOS; другой интерпретатор задаётся так: `make PYTHON_EXE=/path/python3.12 python-test`.

---

## PHP (FFI)

Пакет Composer `bindings/php` (PHP 8.2+, расширение `ffi`). Нативный код вызывается через встроенный в PHP FFI, поэтому ничего не компилируется на машине пользователя и не нужно своё расширение PHP.

```php
use Beresta\{BerestaException, Document, Parser, ParseFlags, Schema, WriteFlags, XPath};

$document = (new Parser())->parseString('<shop><item code="A1">Чай</item></shop>');
$item = $document->root()->firstChild();
echo $item['code'], ' ', $item->text(), PHP_EOL;                    // A1 Чай
echo (new XPath('//item[@code = $c]', [], ['c' => 'A1']))->string($document), PHP_EOL;
foreach ($document->root() as $child) {
    echo $child->name(), PHP_EOL;                                    // item
}

$order = Document::create('order');
$order->root()->appendElement('item')->setAttribute('qty', '2')->appendText('Мёд & орехи');
$order->save('order.xml', WriteFlags::PRETTY, 'windows-1251');

try {
    $schema = Schema::fromFile('order.xsd');
    $checked = (new Parser(ParseFlags::TRACK_LOCATIONS))->parseFile('order.xml');
    foreach ($schema->validate($checked) as $problem) {              // готовое дерево
        echo $problem, PHP_EOL;                                      // "строка:столбец: текст"
    }
    $prices = (new XPath('//item/@price'))->selectValues($checked);  // строки без объектов-узлов
    $export = $schema->validateFile('export.xml');                   // поток без дерева, хоть гигабайты
} catch (BerestaException $error) {
    echo $error->statusName(), ' ', $error->detail(), PHP_EOL;
}
```

Подготовка пакета:

```bash
make PHP=1
```

Как это устроено:
- `resources/beresta.h` — те же 71 функция, что и в `beresta_c.h`, но без препроцессора: разборщик PHP FFI не понимает `#ifdef` и макросы. Совпадение имён проверяет тест `HeaderSyncTest`, а наличие символов — сам `FFI::cdef` при загрузке.
- Возвращаемый `const char*` PHP FFI сам превращает в строку, а `char*` оставляет указателем: обёртка копирует его и сразу освобождает через `beresta_string_free`.
- Документ освобождается при `close()` или деструктором, как только на него и его узлы не осталось ссылок. Подсчёт ссылок PHP делает это сразу, без сборщика мусора. После `close()` обращение к узлу даёт `ClosedException`, а не падение процесса.
- Узлы выглядят как обычные объекты PHP: `count($node)`, `foreach ($node as $child)`, `echo $node` (XML узла). Атрибуты элемента читаются и пишутся и методами в духе ext/dom, и как элементы массива, как в SimpleXML: `$item['code']`, `isset`, `unset`. `clone $document` делает глубокую копию.
- У одного узла дерева всегда один объект PHP (карта обёрток документа со слабыми ссылками). Поэтому `moveTo` в другой документ безопасен и для объектов потомков, полученных раньше.
- Библиотека ищется в `BERESTA_LIBRARY`, затем в `native/<os>-<arch>/` пакета, затем в системе.
- В PHP-FPM и Apache по умолчанию `ffi.enable=preload`, и `FFI::cdef` в веб-запросе запрещён. Для них библиотека загружается один раз в скрипте `opcache.preload` вызовом `Beresta\Beresta::preload()`, а пакет затем берёт готовую область FFI `BERESTA`. Другой путь — `ffi.enable=1` в php.ini.

Самопроверка — свой маленький прогонщик без PHPUnit и `composer install`: `make PHP=1 php-test` (или `composer test` в `bindings/php`). Другой интерпретатор задаётся так: `make PHP_EXE=/path/php8.4 php-test`.

---

## Node.js (Node-API)

Пакет `bindings/node` (Node.js 18+, в Node.js 24+ работает `using`). Вызовы идут через собственный аддон на Node-API, а не через `ffi-napi` или node-gyp: объявления стабильного ABI Node-API написаны вручную, а нужные функции аддон находит в процессе Node при загрузке. Поэтому для установки не нужны ни заголовки Node, ни компилятор, ни npm-зависимости — готовый `beresta.node` (около 1–2 МБ, ядро уже внутри) кладёт в пакет сама сборка. Типы TypeScript входят в пакет (`lib/index.d.ts`).

```js
const beresta = require('beresta');

using document = beresta.parse("<shop><item code='A1'>Чай</item><item code='B2'>Кофе</item></shop>");
const item = document.root.firstChild;
console.log(item.getAttribute('code'), item.text);                          // A1 Чай

using query = new beresta.XPath('//item[@code = $c]', { variables: { c: 'B2' } });
console.log(query.string(document));                                        // Кофе

using order = beresta.createDocument('order');
order.root.appendElement('item').setAttribute('qty', 2).appendText('Мёд & орехи');
console.log(order.toXml(beresta.WriteFlags.PRETTY));
order.save('order.xml', { encoding: 'windows-1251' });

using schema = beresta.Schema.fromFile('order.xsd');
for (const problem of schema.validate(order))
    console.log(problem.line, problem.message);
const exported = schema.validateFile('export.xml');                         // поток без дерева

try { beresta.parse('<a><b></a>'); }
catch (error) { console.log(error.code, error.message); }                   // PARSE_ERROR строка 1, ...
```

Сборка и самопроверка (встроенный `node --test`, без Jest и Mocha):

```bash
make NODE=1 node-test
```
```bash
make NODE=1 node-package
```

Как это устроено:
- Аддон (`addon/`, C++17) — 31 переходник по сигнатурам для 68 функций Си-интерфейса (паттерны Adapter и Command). Он сам превращает коды ошибок в исключения `BerestaError` с кодом (`PARSE_ERROR`) и текстом на текущем языке.
- Дескрипторы — `BigInt`: адрес не обязан помещаться в 53 бита числа, а одинаковые адреса равны по `===`. Две обёртки одного узла сравниваются через `node.isSameNode(other)`.
- Разбор `Buffer` и `Uint8Array` идёт без копирования байтов; строка кодируется в UTF-8.
- `XmlDocument`, `XPath` и `Schema` закрываются через `close()` или `using` (`Symbol.dispose`). Если забыть, память освободит `FinalizationRegistry`.
- Данные аддона хранятся отдельно для каждого окружения (`napi_set_instance_data`), поэтому пакет работает и в `worker_threads`.
- Аддон ищется в `BERESTA_NODE_ADDON`, затем в `native/<os>-<arch>/beresta.node`.
- Пример на TypeScript запускается без компилятора (Node.js 23.6+): `node bindings/node/examples/demo.mts`.
- Пакет из npm содержит аддоны всех платформ (см. [Готовые пакеты и релизы](building.md#готовые-пакеты-и-релизы)); для браузера есть `beresta-wasm` с тем же API.

---

## WebAssembly (браузер и Workers)

Пакет `bindings/wasm` (`beresta-wasm`): ядро, собранное Emscripten, и **тот же JS-слой, что у Node.js** — классы, методы и ошибки совпадают, только API становится доступен после `await init()`.

```js
import init from 'beresta-wasm';

const beresta = await init();
const order = beresta.parse(xmlText);
const schema = beresta.Schema.fromString(xsdText);
for (const problem of schema.validate(order))
    console.log(problem.message);                 // /order/item[3]: атрибут "price": значение "-5" меньше минимума 0
const problems = schema.validateBytes(fileBytes); // поток без дерева: Uint8Array из <input type="file">
console.log(new beresta.XPath('sum(//@price)').number(order));
order.close();
schema.close();
```

Сборка и самопроверка (нужен Emscripten 3.1+):

```bash
make wasm-test
```

Готовая страница-пример — `bindings/wasm/examples/browser.html`: XML, XSD и XPath, которые проверяются прямо в браузере. Откройте её через любой статический сервер (`python -m http.server --directory bindings/wasm`, затем `http://localhost:8000/examples/browser.html`).

Как это устроено:
- Ядро компилируется `em++` с `-fwasm-exceptions` (исключения C++ средствами самого Wasm) и экспортирует те же функции Си-интерфейса, что и динамическая библиотека (`bindings/wasm/exports.json`).
- `lib/native_wasm.js` — бэкенд для общего JS-слоя (паттерн Adapter): та же таблица функций, что у аддона Node.js, а 31 сигнатура описана данными (`wasm_signatures.js`), и один общий вызов кладёт строки и байты в память Wasm, разбирает результат и превращает ошибки в `BerestaError`.
- Слой написан в CommonJS, а браузеру нужен ES-модуль, поэтому `tools/bundle.mjs` — собственный сборщик без зависимостей: он обходит `require`, подменяет загрузчик аддона на бэкенд Wasm и пишет один файл `dist/beresta.mjs`. Классы не дублируются: правка в `bindings/node/lib` сразу попадает и в браузерный пакет.
- Модуль весит около 650 КБ (около 250 КБ в gzip). Разбор — примерно две трети скорости нативного аддона, XPath — не медленнее.
- В Cloudflare Workers модуль передаётся через `init({ instantiateWasm })` (пример — в `bindings/wasm/README.md`); CI проверяет браузер и Node.js, а Workers — нет.
