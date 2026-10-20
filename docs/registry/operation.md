# Руководство по эксплуатации

Документ описывает функциональные характеристики программы для ЭВМ «XML-библиотека „Берёста“ (beresta)» и порядок работы с ней: программный интерфейс на C++ и плоский Си-интерфейс, примеры разбора (DOM и SAX), запросов XPath и проверки по XML Schema, обработку ошибок и безопасность. Установка и сборка описаны в «Руководстве по установке и развёртыванию».

Правообладатель: Ермаков Алексей Александрович.

## Функциональные характеристики

«Берёста» — библиотека, которую встраивают в прикладные программы для работы с документами XML. Пользовательского интерфейса у неё нет: функции вызываются из кода программы. Все сообщения об ошибках выводятся на русском языке (по умолчанию) или на английском.

| Возможность | Стандарт и объём поддержки |
|---|---|
| Разбор XML | XML 1.0 (5-я редакция): 100 % применимых случаев W3C XML Conformance Test Suite |
| Пространства имён | Namespaces in XML: проверка при разборе, URI вычисляются по текущему месту узла в дереве |
| Кодировки | UTF-8, UTF-16 LE/BE, ISO-8859-1, US-ASCII, windows-1251, windows-1252, KOI8-R, IBM866 (собственные таблицы) |
| Дерево документа (DOM) | создание, навигация, изменение, копирование и перенос узлов между документами |
| Потоковый разбор (SAX) | события без построения дерева, память не зависит от размера документа |
| DTD | внутреннее и внешнее подмножества, параметрические сущности, атрибуты по умолчанию, проверка валидности |
| XML Schema | XSD 1.0: 45 встроенных типов, фасеты, наследование, группы замены, `xs:key`/`xs:unique`/`xs:keyref`, `include`/`import`/`redefine`; из XSD 1.1 — `xs:assert`, `xs:assertion`, `xs:override`. W3C XML Schema Test Suite — 96,2 % применимых случаев |
| XPath | XPath 1.0 полностью: 13 осей, 27 функций, переменные и префиксы |
| XInclude | XInclude 1.0: `parse="xml\|text"`, `xi:fallback`, XPointer `element()` и по ID |
| Обход дерева | DOM Level 2 Traversal: `TreeWalker`, `NodeIterator` с фильтрами |
| Запись XML | компактный и форматированный вывод, любая поддерживаемая выходная кодировка |
| Другие языки | Си-интерфейс (71 функция) и обёртки Java 22+, C#, Python, Node.js, WebAssembly |

Сознательные ограничения перечислены в разделе «Ограничения» в конце документа.

## Порядок работы

1. Подключите библиотеку к проекту (см. «Руководство по установке и развёртыванию»).
2. Программа на C++ подключает один заголовок `beresta.hpp`; программа на C или другом языке — `capi/beresta_c.h` или пакет своего языка.
3. Разберите документ (`XmlParser`, `beresta_document_parse`) или создайте его с нуля.
4. Работайте с деревом, вычисляйте XPath, проверяйте по схеме.
5. Запишите результат в строку или файл.
6. Обрабатывайте ошибки: в C++ — исключения `beresta::DomException`, в C — коды состояния и `beresta_last_error_message()`.

## Работа на C++

### Разбор, поиск, изменение, сохранение (DOM)

```cpp
#include "beresta.hpp"
#include <iostream>

int main()
{
    try
    {
        beresta::Document doc = beresta::XmlParser().parseFile("library.xml");

        for (beresta::Element* book : beresta::xpath::selectElements(doc, "//book[@lang='ru']"))
            book->setAttribute("checked", "yes");

        beresta::SerializerOptions options;
        options.prettyPrint = true;
        beresta::XmlSerializer(options).writeFile(doc, "library.out.xml");
    }
    catch (const beresta::DomException& error)
    {
        std::cerr << error.what() << '\n';     // [PARSE_ERROR] строка 3, столбец 7: ...
        return 1;
    }
    return 0;
}
```

### Построение документа с нуля

```cpp
beresta::Document doc;
doc.setDeclaration(beresta::XmlDeclaration{"1.0", "UTF-8", std::nullopt});

beresta::Element* root = doc.setRoot(std::make_unique<beresta::Element>("note"));
root->setAttribute("title", "Q&A");
root->appendChild(std::make_unique<beresta::Comment>(" черновик "));

auto body = std::make_unique<beresta::Element>("body");
body->setTextContent("1 < 2");
root->appendChild(std::move(body));

std::cout << doc.toString();
// <?xml version="1.0" encoding="UTF-8"?><note title="Q&amp;A"><!-- черновик --><body>1 &lt; 2</body></note>
```

Родитель владеет детьми (`std::unique_ptr`), поэтому освобождать узлы вручную не нужно. Основные настройки разбора — структура `ParserOptions`: `preserveWhitespaceText`, `discardComments`, `trackLocations`, `validateDtd`, `processXInclude`, лимиты `maxDepth` (1024), `maxEntityExpansions` (10 000), `maxExpandedSize` (10 МБ), `resolver`, `errorHandler`, `schema`.

### Потоковый разбор (SAX)

Дерево не строится, расход памяти не зависит от размера документа:

```cpp
class PriceCounter : public beresta::ContentHandler
{
public:
    void startElement(const beresta::SaxElement& e) override { inside = e.localName == "price"; }
    void characters(std::string_view text) override { if (inside) total += beresta::xpath::stringToNumber(text); }
    void endElement(const beresta::SaxElement&) override { inside = false; }

    bool inside = false;
    double total = 0;
};

PriceCounter counter;
beresta::SaxParser(options).parseFile("big.xml", counter);
```

События: `startDocument`/`endDocument`, `xmlDeclaration`, `documentType`, `startElement`/`endElement`, `characters`, `cdata`, `ignorableWhitespace`, `comment`, `processingInstruction`. Строки `string_view` в событиях действительны только на время вызова.

### Запросы XPath 1.0

```cpp
// Быстрые функции
std::vector<beresta::Element*> books = beresta::xpath::selectElements(doc, "//book[price > 300]");
std::string title = beresta::xpath::evaluateString(doc, "string(//book[1]/title)");

// Скомпилированное выражение: многократно и из разных потоков
beresta::xpath::XPathExpression expression("//s:item[@code = $code]/s:name");
beresta::xpath::XPathContext context;
context.namespaces.emplace("s", "urn:shop");
context.variables.emplace("code", beresta::xpath::XPathValue::fromString("B2"));

beresta::xpath::XPathValue value = expression.evaluate(doc, context);
value.getType();          // NODE_SET, BOOLEAN, NUMBER, STRING
```

### Проверка по XML Schema

```cpp
// Загрузка схемы (один раз; схема неизменяема и разделяется между потоками)
std::shared_ptr<const beresta::xsd::Schema> schema = beresta::xsd::Schema::fromFile("catalog.xsd", loadOptions);

// Вариант 1: проверка готового дерева
beresta::xsd::SchemaValidator validator(schema);
std::vector<beresta::Diagnostic> errors = validator.validate(doc);

// Вариант 2: проверка при разборе в DOM — один проход
beresta::ParserOptions options;
options.schema = schema;
options.errorHandler = &handler;          // без него — исключение на первой ошибке
beresta::XmlParser(options).parseFile("catalog.xml");

// Вариант 3: потоковая проверка без дерева — для выгрузок в гигабайты
MyHandler events;                         // любой ContentHandler
beresta::SaxParser(options).parseFile("export.xml", events);
```

Каждое нарушение содержит позицию и путь к элементу, например `строка 12, столбец 5: /catalog/book[2]/price: значение "-5" меньше минимума 0`. Во всех трёх вариантах нарушения, их порядок и позиции совпадают.

## Работа через Си-интерфейс

Заголовок `include/capi/beresta_c.h` написан на C99 и подключается и в `.c`, и в `.cpp`. На нём построены обёртки Java, C#, Python и Node.js; им же пользуются из C, Go (cgo), Rust (FFI), Delphi и любого языка, умеющего вызывать функции C.

### Соглашения

| Правило | Пояснение |
|---|---|
| Дескрипторы | `beresta_document`, `beresta_node`, `beresta_xpath`, `beresta_nodeset`, `beresta_schema`, `beresta_diagnostics` — непрозрачные указатели `void*` |
| Статус | функции, возвращающие `int`, отдают `BERESTA_OK` (0) при успехе или код ошибки |
| NULL | функции, возвращающие дескриптор или `char*`, отдают NULL при ошибке; NULL с кодом 0 означает «объекта нет» |
| Последняя ошибка | `beresta_last_error_code()` и `beresta_last_error_message()` — своя у каждого потока; каждый вызов её сбрасывает |
| Строки | UTF-8; `char*` от библиотеки освобождается через `beresta_string_free`, `const char*` — заимствованная |
| Размеры и индексы | `int64_t`; `-1` в результате означает ошибку |
| Владение | документ, схему, выражение, набор узлов и список нарушений освобождает вызывающий (`beresta_*_free`); узел принадлежит документу |
| Потоки | разные документы — в разных потоках параллельно; схема и скомпилированное выражение потокобезопасны |

### Пример: DOM и XPath на C

```c
#include "capi/beresta_c.h"
#include <stdio.h>
#include <string.h>

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

### Пример: построение документа на C

```c
beresta_document document = NULL;
beresta_node item;

if (beresta_document_create("order", &document) != BERESTA_OK)
    return 1;
item = beresta_node_append_element(beresta_document_root(document), "item");
beresta_element_set_attribute(item, "qty", "2");
beresta_node_append_text(item, "Мёд & орехи");

char* text = beresta_node_to_string(document, BERESTA_WRITE_PRETTY);
printf("%s\n", text);
beresta_string_free(text);
beresta_document_free(document);
```

### Пример: потоковая проверка файла по схеме на C

```c
beresta_schema schema = NULL;
beresta_diagnostics problems = NULL;

if (beresta_schema_load_file("catalog.xsd", &schema) != BERESTA_OK)
    return 1;
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
beresta_schema_free(schema);
```

Полный пример на C — файл `examples/capi/capi_demo.c` (собирается вместе с библиотекой как `beresta_capi_demo`).

## Справочник Си-интерфейса

<!-- capi-reference -->

## Обёртки для других языков

| Язык | Установка | Основа |
|---|---|---|
| Java 22+ | Maven `io.github.ermaleyalekov:beresta` | Project Panama (Foreign Function & Memory API), без JNI |
| C# | `dotnet add package Beresta` | P/Invoke и `SafeHandle` |
| Python 3.8+ | `pip install beresta` | ctypes, без компилятора на машине пользователя |
| Node.js 18+ | `npm install beresta` | собственный модуль Node-API, типы TypeScript в комплекте |
| Браузер, Workers | `npm install beresta-wasm` | WebAssembly (Emscripten), тот же JS-слой |

Обёртки повторяют возможности Си-интерфейса: DOM, XPath, XML Schema, включая потоковую проверку без дерева. Подробно — в файле `docs/bindings.md` репозитория.

## Обработка ошибок

В C++ все ошибки — исключения `beresta::DomException` (код и сообщение) и производное `beresta::ParseException` (с позицией: смещение, строка, столбец). Коды ошибок `DomErrorCode`: `PARSE_ERROR`, `NAMESPACE_ERROR`, `ENCODING_ERROR`, `VALIDATION_ERROR`, `RESOURCE_ERROR`, `LIMIT_EXCEEDED`, `SCHEMA_ERROR`, `XPATH_ERROR`, `INVALID_NAME`, `INVALID_CONTENT`, `HIERARCHY_VIOLATION`, `INDEX_OUT_OF_RANGE`, `NULL_NODE`, `NOT_FOUND`, `IO_ERROR`.

```cpp
try { /* ... */ }
catch (const beresta::ParseException& e) { e.getLine(); e.getColumn(); e.getCode(); }
catch (const beresta::DomException& e)   { e.getCode(); e.getMessage(); beresta::errorCodeToString(e.getCode()); }
```

Нефатальные ошибки (нарушения DTD и XSD) можно собирать обработчиком `ErrorHandler` (готовый — `CollectingErrorHandler`), не прерывая разбор. В Си-интерфейсе исключения не пересекают границу библиотеки: каждая функция возвращает код состояния (таблица «Коды состояния» выше), а текст — `beresta_last_error_message()`.

### Язык сообщений

Язык по умолчанию задаётся при сборке (`make ERRORS_LANG=en`, `-DBERESTA_ERRORS_LANG=en`; без параметра — русский) и меняется во время работы одним вызовом для всех потоков: `beresta::i18n::setLanguage("en")` в C++, `beresta_set_language("en")` в C. Имя кода ошибки одинаково во всех языках:

```
[PARSE_ERROR] строка 1, столбец 7: ожидался </b>, найден </a>
[PARSE_ERROR] line 1, column 7: expected </b>, found </a>
```

## Безопасность

- По умолчанию **ничего внешнего не загружается**: ни внешний DTD, ни внешние сущности, ни XInclude, ни `xs:include`. Это закрывает атаки XXE и SSRF. Загрузку разрешает только явно переданный загрузчик (`FileResourceResolver` — только локальные файлы внутри заданного каталога, без сетевых адресов) или флаг `BERESTA_PARSE_FILE_RESOURCES` в Си-интерфейсе.
- Лимиты `maxEntityExpansions`, `maxEntityDepth`, `maxExpandedSize` и `maxDepth` всегда включены и защищают от «сущностных бомб» и слишком глубоких документов; превышение — ошибка `LIMIT_EXCEEDED`.
- Регулярные выражения XSD выполняются за линейное время; хеш-таблицы с ключами из документа защищены от подбора коллизий (SipHash со случайным ключом процесса).
- Рекурсии по дереву нет: разбор, копирование, вывод, проверка и удаление дерева обходят его по явному стеку.

## Потокобезопасность

- `XmlParser`, `SaxParser`, `XmlSerializer`, `XPathExpression`, `Schema`, `SchemaValidator` неизменяемы: один объект можно использовать из многих потоков одновременно.
- Разные деревья обрабатываются параллельно без синхронизации; одно дерево можно читать из многих потоков, а при изменении нужна внешняя синхронизация.

## Ограничения

- XML Schema: при загрузке схемы не проверяются корректность ограничения модели содержимого и однозначность модели (UPA); документ схемы проверяется по схеме для схем не целиком; из XSD 1.1 поддерживаются только `xs:assert`, `xs:assertion`, `xs:override` и `vc:minVersion`/`vc:maxVersion`.
- XPath: XPath 1.0 полностью; из XPath 2.0 — только подмножество для утверждений XSD 1.1.
- Нет XSLT, DOM Range, особенностей XML 1.1 и загрузки по сети.
- Через Си-интерфейс и обёртки доступны DOM, XPath и XML Schema; собственный SAX-обработчик, `TreeWalker` и свой загрузчик ресурсов — только из C++.
- В WebAssembly нет файловой системы: работа идёт со строками и байтами.

Полный список — в файле `docs/comparison.md` репозитория (раздел «Ограничения»).
