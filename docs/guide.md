# Берёста — руководство и примеры на C++

**Русский** · [English](guide.en.md)

← [К описанию и преимуществам](../README.md)

## Содержание

1. [Быстрый старт](#быстрый-старт)
2. [Модель данных и память](#модель-данных-и-память)
3. [Работа с деревом](#работа-с-деревом)
4. [Копирование и перенос узлов](#копирование-и-перенос-узлов)
5. [Разбор: DOM](#разбор-dom)
6. [Разбор: SAX](#разбор-sax)
7. [Пространства имён](#пространства-имён)
8. [Кодировки, файлы и потоки](#кодировки-файлы-и-потоки)
9. [Сериализация](#сериализация)
10. [DTD и сущности](#dtd-и-сущности)
11. [XML Schema](#xml-schema)
12. [XPath 1.0](#xpath-10)
13. [Обход дерева: TreeWalker и NodeIterator](#обход-дерева-treewalker-и-nodeiterator)
14. [XInclude](#xinclude)
15. [Внешние ресурсы и безопасность](#внешние-ресурсы-и-безопасность)
16. [Обработка ошибок](#обработка-ошибок)
17. [Языки сообщений](#языки-сообщений)
18. [Гарантии и потокобезопасность](#гарантии-и-потокобезопасность)

---

## Быстрый старт

### Разобрать, найти, изменить, сохранить

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

### Построить документ с нуля

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

> Если в консоли Windows кириллица выводится «кракозябрами», переключите консоль в UTF-8: `SetConsoleOutputCP(CP_UTF8)` из `<windows.h>` (так сделано в `examples/` и `tests/`).

---

## Модель данных и память

### Иерархия классов (паттерн Composite)

```
Node                          абстрактный узел
├── ContainerNode             узел с детьми (абстрактный)
│   ├── Element               тег, атрибуты, дети
│   └── Document              декларация, DOCTYPE, пролог, один корень
├── CharacterNode             узел со строковым значением (абстрактный)
│   ├── Text
│   ├── Comment
│   └── CDataSection
├── ProcessingInstruction     <?target data?>
└── DocumentType              <!DOCTYPE ...> + модель DTD
```

`NodeType`: `DOCUMENT`, `DOCUMENT_TYPE`, `ELEMENT`, `TEXT`, `CDATA_SECTION`, `COMMENT`, `PROCESSING_INSTRUCTION`.

XML-декларация — не узел, а свойство документа (`Document::getDeclaration()`). Цель PI `xml` зарезервирована: `ProcessingInstruction("xml", ...)` бросает `INVALID_NAME`.

### Владение

- Родитель **владеет** детьми (`std::vector<std::unique_ptr<Node>>`).
- Ребёнок **знает** родителя через невладеющий указатель, а свой индекс среди соседей хранит сам. Поэтому `getNextSibling()` работает за O(1).
- Владение направлено только сверху вниз: циклов нет, и удаление корня освобождает всё поддерево.

**Почему не `shared_ptr` + `weak_ptr`:** тогда каждый узел пришлось бы создавать в `shared_ptr`, и обычное `Element copy = original;` на стеке не могло бы привязать детей к копии.

| Откуда указатель | Сколько он живёт |
|---|---|
| `appendChild`, `getChild`, `findChildElement`, `getRoot`, XPath, `TreeWalker` | пока узел в дереве и дерево живо |
| `removeChild`, `detachChild`, `replaceChild`, `releaseRoot`, `clone`, `parseElement` | вы владеете узлом (`unique_ptr`) |

---

## Работа с деревом

### Навигация

```cpp
beresta::Element* root = doc.getRoot();
beresta::Node* first = root->getFirstChild();
beresta::Node* next = first->getNextSibling();          // O(1)
beresta::Node* last = root->getLastChild();
std::size_t index = next->getIndexInParent();
beresta::Document* owner = next->getOwnerDocument();
bool inside = next->isDescendantOf(*root);
```

### Изменение

```cpp
root->appendChild(std::make_unique<beresta::Element>("item"));
root->insertChild(0, std::make_unique<beresta::Comment>("начало"));
std::unique_ptr<beresta::Node> old = root->replaceChild(1, std::make_unique<beresta::Text>("новое"));
std::unique_ptr<beresta::Node> taken = root->detachChild(*root->getLastChild());
root->normalize();                 // слить соседние Text, удалить пустые
root->setTextContent("только текст");
root->clearChildren();
```

### Атрибуты

```cpp
item.setAttribute("id", "42");                       // добавить или заменить (позиция сохраняется)
std::optional<std::string> id = item.getAttribute("id");
bool has = item.hasAttribute("id");
bool removed = item.removeAttribute("id");
for (const beresta::Attribute& a : item.getAttributes()) { /* a.name, a.value */ }
item.setAttributes({{"a", "1"}, {"b", "2"}});        // пакетом за O(n); имена проверяются до изменений
```

### Поиск

```cpp
const beresta::Element* first = root->findChildElement("item");                 // среди детей
std::vector<const beresta::Element*> direct = root->findChildElements("item");
std::vector<const beresta::Element*> deep = doc.getElementsByTagName("item");    // всё поддерево, "*" — любые
const beresta::Element* byId = doc.getElementById("b7");                         // ID из DTD или xml:id
```

Для сложных запросов используйте [XPath](#xpath-10).

### Сравнение и текст

```cpp
bool same = a.isEqualNode(b);               // структурное равенство; порядок атрибутов не важен
std::string text = element.getTextContent(); // текст потомков (Text и CDATA, без комментариев)
```

---

## Копирование и перенос узлов

```cpp
beresta::Element copy = *book;                                  // глубокая копия на стеке
auto heap = std::make_unique<beresta::Element>(*book);          // в куче
std::unique_ptr<beresta::Node> any = node->clone();             // полиморфно, тип сохраняется
std::unique_ptr<beresta::Element> shell = book->cloneShallow(); // имя и атрибуты, без детей

*target = source;     // содержимое заменяется глубокой копией, место target в дереве не меняется
```

Копия **автономна** (`getParent() == nullptr`) и не делит с оригиналом ни одного узла. Перенос без копирования:

```cpp
targetParent->appendChild(sourceParent->removeChild(index));
```

Узел, извлечённый из документа, переживает сам документ.

Из других языков то же самое делают `beresta_node_move_to` и методы обёрток рядом с копированием:

| Язык | Копия | Перенос без копирования |
|---|---|---|
| C | `beresta_node_copy_to(node, parent, &copy)` | `beresta_node_move_to(node, parent)` |
| Java | `node.copyTo(parent)` | `node.moveTo(parent)` |
| C# | `node.CopyTo(parent)` | `node.MoveTo(parent)` |
| Python | `node.copy_to(parent)` | `node.move_to(parent)` |
| Node.js, WebAssembly | `node.copyTo(parent)` | `node.moveTo(parent)` |

```python
source = beresta.parse("<a><item><name>Мёд</name></item></a>")
target = beresta.parse("<b/>")
item = source.root.first_child
name = item.first_child          # обёртка потомка, полученная до переноса
item.move_to(target.root)        # узел (с потомками) теперь принадлежит target
source.close()                   # закрытие прежнего документа узел не трогает
print(name.text, item.document is target)   # Мёд True
```

Перенос ставит узел в конец детей `parent` — элемента или документа (в документе без корня элемент становится корнем). Дескриптор узла и дескрипторы его потомков не меняются: объект не копируется, им просто начинает владеть новый документ. В обёртках у каждого узла одна обёртка (реестр документа), поэтому документ-владелец меняется у всех уже выданных обёрток поддерева: после переноса их срок жизни определяет новый документ, а не прежний. Нельзя перенести сам документ (`INVALID_ARGUMENT`), корневой элемент документа и узел внутрь самого себя или своего потомка (`HIERARCHY_VIOLATION`). Если новый родитель не принимает узел (например, текст на верхний уровень документа), возвращается `HIERARCHY_VIOLATION`, а дерево остаётся прежним.

---

## Разбор: DOM

```cpp
beresta::XmlParser parser(options);
beresta::Document a = parser.parse(text);                   // из памяти
beresta::Document b = parser.parseFile("data.xml");         // из файла (baseUri = путь)
beresta::Document c = parser.parse(stream);                 // std::istream
std::unique_ptr<beresta::Element> e = parser.parseElement("<x a='1'/>");   // автономный фрагмент
```

Вход — **байты** в любой поддерживаемой кодировке: она определяется по BOM и XML-декларации.

### `ParserOptions`

| Поле | По умолчанию | Назначение |
|---|---|---|
| `preserveWhitespaceText` | `false` | сохранять текст из одних пробелов (отступы) |
| `discardComments` | `false` | не добавлять комментарии в DOM |
| `coalesceCData` | `false` | превращать CDATA в обычный текст |
| `trackLocations` | `false` | записывать строку и столбец каждого узла |
| `namespaceAware` | `true` | проверять правила Namespaces in XML |
| `validateDtd` | `false` | проверять документ по DTD |
| `loadExternalDtd` | `false` | загружать внешний DTD (нужен `resolver`) |
| `applyDtdDefaults` | `true` | добавлять атрибуты по умолчанию из `<!ATTLIST>` |
| `processXInclude` | `false` | выполнять `xi:include` (нужен `resolver`) |
| `maxDepth` | `1024` | максимальная вложенность элементов |
| `maxEntityExpansions` | `10000` | максимальное число раскрытий сущностей |
| `maxEntityDepth` | `16` | вложенность сущностей (и XInclude) |
| `maxExpandedSize` | 10 МБ | суммарный объём текста из сущностей |
| `forcedEncoding` | `""` | принудительная кодировка входа |
| `baseUri` | `""` | база для относительных ссылок |
| `resolver` | `nullptr` | загрузчик внешних ресурсов |
| `errorHandler` | `nullptr` | получатель ошибок и предупреждений |
| `schema` | `nullptr` | XML Schema для проверки во время разбора (DOM и SAX) |

`resolver` и `errorHandler` — **невладеющие** указатели: объекты должны жить, пока идёт разбор.

### Пробелы

Текст **только из пробелов** между тегами по умолчанию отбрасывается: это форматирование, и благодаря этому «красивый» и «сплошной» XML дают одно и то же дерево. Текст, в котором есть хоть один непробельный символ или символьная ссылка, сохраняется дословно. Для смешанной разметки (`<b>a</b> <i>b</i>`) включите `preserveWhitespaceText`.

### Позиции узлов

```cpp
beresta::ParserOptions options;
options.trackLocations = true;
beresta::Document doc = beresta::XmlParser(options).parse(text);
std::size_t line = doc.getRoot()->getSourceLocation().line;
```

Узел из раскрытой сущности получает позицию ссылки на эту сущность в исходном файле.

---

## Разбор: SAX

Потоковый разбор: дерево не строится, расход памяти не зависит от размера документа.

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

| Событие | Когда |
|---|---|
| `setDocumentLocator(const Locator&)` | до начала; `locator.getLocation()` даёт позицию текущего события |
| `startDocument()` / `endDocument()` | начало и конец |
| `xmlDeclaration(const XmlDeclaration&)` | декларация |
| `documentType(const DocumentType&)` | DOCTYPE вместе с моделью DTD |
| `startElement(const SaxElement&)` / `endElement(...)` | теги: `name`, `namespaceUri`, `localName`, `attributes` |
| `characters(text)` / `cdata(text)` | текст (сущности уже раскрыты) / CDATA |
| `ignorableWhitespace(text)` | пробелы между тегами |
| `comment(text)` / `processingInstruction(target, data)` | комментарии и PI |
| `dtdProcessingInstruction(target, data)` | PI из внутреннего и внешнего DTD, до `documentType`; по умолчанию вызывает `processingInstruction` (раздел 2.6: PI передаются приложению). `DomBuilder` их пропускает |

Все `string_view` в событиях действительны **только на время вызова**. `DomBuilder` — открытый обработчик, строящий DOM; его можно унаследовать.

С `options.schema` документ по ходу разбора проверяется по XML Schema потоковым валидатором, а ваш обработчик получает те же события; дерево при этом не строится (см. [XML Schema](#xml-schema)).

---

## Пространства имён

При разборе (`namespaceAware = true`) проверяется: префиксы объявлены, `xml`/`xmlns` не переопределены, имена корректны, нет двух атрибутов с одним расширенным именем. Нарушение даёт фатальную ошибку `NAMESPACE_ERROR`.

В DOM URI вычисляются **«вживую»** по объявлениям `xmlns` в предках. Поэтому они всегда соответствуют тому, что будет записано в файл, даже после переноса элемента.

```cpp
const beresta::Element* body = envelope->findChildElementNS("http://schemas.xmlsoap.org/soap/envelope/", "Body");
body->getPrefix();                  // "soap"
body->getLocalName();               // "Body"
body->getNamespaceURI();            // optional<string>
body->lookupNamespaceURI("soap");
body->lookupPrefix("urn:x");
doc.getElementsByTagNameNS("urn:x", "*");

element.declareNamespace("x", "urn:x");             // xmlns:x="urn:x"
element.setAttributeNS("urn:x", "x:flag", "1");     // объявит префикс сам, если нужно
element.getAttributeNS("urn:x", "flag");
element.removeAttributeNS("urn:x", "flag");
```

---

## Кодировки, файлы и потоки

Определение кодировки входа:
1. BOM (UTF-8, UTF-16LE, UTF-16BE).
2. Сигнатура `<?` в UTF-16 без BOM.
3. Атрибут `encoding` в декларации (`<?xml` и сразу пробел; `<?xml-stylesheet ...?>` — обычная PI).
4. Иначе UTF-8.

Если кодировку определили BOM или сигнатура UTF-16, а декларация называет другую (BOM UTF-8 и `encoding="iso-8859-1"`), это ошибка `ENCODING_ERROR` (раздел 4.3.3). Концы строк (`\r\n`, `\r`) нормализуются один раз — при декодировании документа, внешнего DTD или сущности; символ `\r` из ссылки `&#13;` сохраняется.

`ParserOptions::forcedEncoding` перекрывает автоопределение (и проверку согласия с декларацией). Байты, недопустимые в кодировке, и символы, запрещённые в XML (управляющие коды и т.п.), дают ошибку `ENCODING_ERROR` с позицией.

| Кодировка | Имена |
|---|---|
| UTF-8 | `UTF-8`, `utf8` |
| UTF-16 | `UTF-16` (с BOM), `UTF-16LE`, `UTF-16BE` |
| Латиница | `ISO-8859-1` (`latin1`), `US-ASCII`, `windows-1252` (`cp1252`) |
| Кириллица | `windows-1251` (`cp1251`), `KOI8-R`, `IBM866` (`cp866`) |

```cpp
const beresta::encoding::Charset* cp1251 = beresta::encoding::findCharset("cp1251");

std::string bytes = beresta::io::readFile("in.xml");    // двоичное чтение, путь — std::filesystem::path
beresta::io::writeFile("out.xml", bytes);
beresta::io::readStream(std::cin);
```

---

## Сериализация

```cpp
std::string utf8 = node.toString();     // компактно, всегда UTF-8
node.serialize(buffer);                 // дописать в существующий буфер

beresta::SerializerOptions options;
options.prettyPrint = true;              // отступы
options.indent = "  ";
options.newline = "\n";
options.encoding = "windows-1251";       // кодировка результата
options.writeDeclaration = true;         // писать декларацию (если она была или кодировка не UTF-8)
options.forceDeclaration = false;        // писать даже для документа без декларации

beresta::XmlSerializer serializer(options);
std::string bytes = serializer.toString(doc);
serializer.write(doc, stream);
serializer.writeFile(doc, "out.xml");
```

Правила:
- Компактный вывод не содержит ни одного лишнего пробела, а порядок атрибутов сохраняется.
- При `prettyPrint` каждый ребёнок элемента, содержащего только элементы, идёт с новой строки. Элементы с текстом выводятся дословно, поэтому повторный разбор даёт то же дерево.
- `encoding` в декларации всегда соответствует фактической кодировке вывода. Для `UTF-16` пишется BOM. Символы вне кодировки становятся `&#N;`.
- DOCTYPE выводится в минимальной форме: `<!DOCTYPE r SYSTEM "r.dtd"[...]>`.

---

## DTD и сущности

```cpp
const char* xml =
    "<!DOCTYPE r [<!ENTITY who \"мир\"><!ENTITY bold \"<b>&who;</b>\">"
    "<!ATTLIST r lang CDATA \"ru\">]><r>Привет, &bold;</r>";
beresta::Document doc = beresta::XmlParser().parse(xml);
doc.getRoot()->getAttribute("lang");      // "ru" — значение по умолчанию из ATTLIST
doc.toString();                            // DOCTYPE сохраняется, сущности раскрыты
```

- **Сущности** — внутренние (с разметкой внутри), внешние (через `resolver`), параметрические `%pe;`, в том числе внутри объявлений во внешнем подмножестве. Работают условные секции `INCLUDE`/`IGNORE`.
- **Атрибуты по умолчанию** и нормализация значений не-CDATA атрибутов выполняются как в Xerces.
- **Модель DTD** доступна через `doc.getDocumentType()->getModel()`: объявления сущностей, элементов, атрибутов и нотаций.
- **Внешняя PE во внутреннем подмножестве** читается по правилам внешнего подмножества (ссылки `%pe;` внутри объявлений, условные секции); относительный `SYSTEM` сущности разрешается от того текста, где она объявлена.
- **Необъявленная сущность.** В документе без внешнего DTD и без ссылок `%pe;` (или со `standalone="yes"`) это фатальная ошибка; иначе — ошибка валидности (если проверка включена), а ссылка пропускается (раздел 4.1). При `standalone="yes"` ссылаться на сущность, объявленную во внешнем DTD, нельзя.
- **Проверка валидности** (`validateDtd = true`) проверяет:
  - корень и объявленность элементов и атрибутов;
  - `#REQUIRED` и `#FIXED`;
  - типы `ID`, `IDREF(S)`, `NMTOKEN(S)`, `ENTITY(IES)`, перечисления, уникальность ID и цели IDREF; с пространствами имён значения ID/IDREF/ENTITY не содержат `:`;
  - модели `EMPTY` (внутри нельзя даже комментарий, PI и ссылку на пустую сущность), `ANY`, `MIXED` и `CHILDREN`;
  - сами объявления: повторное объявление элемента, повтор имени в смешанном содержимом, один ID- и один NOTATION-атрибут на элемент, умолчание ID-атрибута, объявленность нотаций, повторы в перечислениях, лексическая допустимость значений по умолчанию;
  - вложенность скобок групп, объявлений и условных секций в параметрических сущностях;
  - `standalone="yes"`: результат не должен зависеть от внешнего DTD (умолчания, нормализация, пробелы в элементном содержимом).

```cpp
beresta::CollectingErrorHandler handler;
beresta::ParserOptions options;
options.validateDtd = true;
options.errorHandler = &handler;
beresta::XmlParser(options).parse(text);
for (const beresta::Diagnostic& d : handler.getDiagnostics())
    std::cout << d.message << '\n';       // все ошибки за один проход
```

Без `errorHandler` первая ошибка валидности бросает `ParseException` с кодом `VALIDATION_ERROR`.

---

## XML Schema

```cpp
// Загрузка схемы (один раз; схема неизменяема и разделяется между потоками)
std::shared_ptr<const beresta::xsd::Schema> schema = beresta::xsd::Schema::fromFile("catalog.xsd", loadOptions);

// Вариант 1: отдельная проверка готового DOM
beresta::xsd::SchemaValidator validator(schema);
std::vector<beresta::Diagnostic> errors = validator.validate(doc);
bool ok = validator.isValid(doc);

// Вариант 2: проверка прямо при разборе в DOM — один проход, без второго обхода дерева
beresta::ParserOptions options;
options.schema = schema;
options.errorHandler = &handler;          // без него — исключение на первой ошибке
beresta::XmlParser(options).parseFile("catalog.xml");

// Вариант 3: потоковая проверка без дерева — для выгрузок в гигабайты
MyHandler events;                         // любой ContentHandler (или пустой)
beresta::SaxParser(options).parseFile("export.xml", events);
```

Каждая ошибка содержит путь к элементу и позицию: `строка 12, столбец 5: /catalog/book[2]/price: значение "-5" меньше минимума 0`. Позиции ошибок есть всегда: валидатор берёт их у парсера, а узлам дерева они записываются, только если включён `trackLocations`.

**Проверка во время разбора.** Валидатор — декоратор над потоком событий SAX (`xsd::ValidatingHandler`): события проходят через него и уходят дальше — в `DomBuilder`, в ваш обработчик или никуда. Модель содержимого сверяется пошагово, по одному ребёнку (жадный проход для последовательностей из разных элементов, автомат на конфигурациях частиц для остальных моделей), простые значения — при закрытии элемента по накопленному тексту. Все три варианта — одна и та же логика проверки: `SchemaValidator::validate` превращает готовое дерево в поток событий обходом без рекурсии. Поэтому ошибки, их порядок, пути и позиции во всех трёх вариантах совпадают.

- **DOM со схемой** (`XmlParser`): документ проверяется за тот же проход, что и строится; ключи `xs:key`/`xs:unique`/`xs:keyref` — по готовому дереву, тем же движком XPath. С `processXInclude` документ проверяется уже после подстановки включений, по дереву.
- **SAX со схемой** (`SaxParser`): дерево не строится, память проверки — O(глубины документа) плюс значения ID и таблицы ключей. Ключи вычисляются потоково (см. ниже). Ошибки приходят в `errorHandler` после `endDocument`: номера в путях вида `item[5]` известны, только когда дочитаны все соседи.
- **Свой конвейер:** `ValidatingHandler` можно поставить перед любым обработчиком вручную.

```cpp
beresta::DomBuilder builder(options);
beresta::xsd::ValidatingHandler validator(schema, &builder,
    beresta::xsd::validationSettingsFor(options, beresta::xsd::IdentityEvaluation::ON_TREE));
beresta::SaxParser(parserOptionsWithoutSchema).parse(text, validator);
beresta::Document doc = builder.takeDocument();
validator.checkIdentity(doc);                                // ключи — по построенному дереву
std::vector<beresta::Diagnostic> errors = validator.takeDiagnostics();
```

| Загрузка | Описание |
|---|---|
| `Schema::fromString(text, options)` | из текста XSD |
| `Schema::fromFile(path, options)` | из файла (`baseUri` = путь) |
| `Schema::fromDocuments({&a, &b}, options)` | несколько схем разных пространств имён |
| `SchemaLoadOptions::resolver` | загрузчик для `xs:include` / `xs:import` |

**Поддерживается:**
- `targetNamespace`, `elementFormDefault`/`attributeFormDefault`, `form`;
- глобальные и локальные элементы и атрибуты, `ref`, анонимные типы;
- `complexType` с `sequence`/`choice`/`all`, вложенными группами, `minOccurs`/`maxOccurs`, `mixed`;
- `complexContent` `extension`/`restriction`, `simpleContent` `extension`/`restriction`;
- `xs:group`, `xs:attributeGroup`, `xs:any` (`##any`, `##other`, `##local`, `##targetNamespace`, список; `strict`/`lax`/`skip`), `xs:anyAttribute`;
- атрибуты: `use="required|optional|prohibited"`, `default`, `fixed`;
- элементы: `default`, `fixed`, `nillable` + `xsi:nil`, `abstract`, `xsi:type`;
- группы замены `substitutionGroup`: члены (в том числе транзитивные) стоят на месте головы и проверяются по своему объявлению; абстрактные члены и заблокированные `block` в группу не входят;
- наследование: тип из `xsi:type` обязан быть выведен из объявленного (цепочки встроенных типов, `extension`/`restriction`, члены `union`), `block`/`blockDefault` у элемента и типа запрещают способы замены, `final`/`finalDefault` запрещают наследование при загрузке схемы; абстрактный тип без производного `xsi:type` — ошибка;
- `simpleType` `restriction`/`list`/`union`;
- фасеты `length`, `minLength`, `maxLength`, `pattern`, `enumeration`, `whiteSpace`, `minInclusive`, `maxInclusive`, `minExclusive`, `maxExclusive`, `totalDigits`, `fractionDigits`;
- все **45 встроенных типов** XSD 1.0 (строковые, целые с точными границами, `decimal`, `float`/`double`, даты и время с часовыми поясами, `duration`, `hexBinary`, `base64Binary`, `QName`, ID/IDREF(S), NMTOKEN(S), ENTITY(IES) и т.д.);
- уникальность `ID` и существование целей `IDREF`;
- ограничения идентичности `xs:unique`, `xs:key`, `xs:keyref` (подробности ниже);
- `xs:include` (включая «хамелеонные» схемы), `xs:import`, `xs:redefine` и `xs:override` (из XSD 1.1: подмена компонента целиком);
- утверждения XSD 1.1: `xs:assert` у составных типов (унаследованные через `extension`/`restriction` тоже проверяются) и фасет `xs:assertion` у простых типов с `$value` (подробности ниже);
- условное включение XSD 1.1: «Берёста» — процессор версии 1.1, компоненты с `vc:minVersion` > 1.1 или `vc:maxVersion` ≤ 1.1 пропускаются при загрузке.

**Группы замены и `xs:redefine` не замедляют проверку.** Подмена компонентов через `xs:redefine` / `xs:override` происходит в реестре определений до построения моделей — проверяемый документ о ней не знает. Множество допустимых заменителей каждой головы вычисляется один раз при загрузке схемы (транзитивно, с учётом `abstract`, `block` и `final`) и хранится упорядоченным: имя ребёнка ищется двоичным поиском, только если оно не совпало с самой частицей. Признак группы стоит на частице модели, поэтому модели без групп замены по-прежнему идут быстрым жадным путём.

```xml
<!-- v2.xsd: новая версия отраслевой схемы без правки исходной -->
<xs:redefine schemaLocation="v1.xsd">
    <xs:complexType name="Party">
        <xs:complexContent>
            <xs:extension base="Party">          <!-- base — прежний Party из v1.xsd -->
                <xs:sequence><xs:element name="inn" type="xs:string"/></xs:sequence>
            </xs:extension>
        </xs:complexContent>
    </xs:complexType>
</xs:redefine>
```

### Утверждения XSD 1.1: xs:assert и xs:assertion

Правила, которые не выразить моделью содержимого, — сверка сумм, зависимости между полями, условия на даты — записываются прямо в схеме выражением XPath. Пример — банковская выписка, где конечный баланс обязан сойтись с транзакциями:

```xml
<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema"
           xmlns:vc="http://www.w3.org/2007/XMLSchema-versioning" vc:minVersion="1.1">
  <xs:element name="Statement">
    <xs:complexType>
      <xs:sequence>
        <!-- AccountInfo: saldoAwal, saldoAkhir (xs:decimal); Transactions/Tx: type, amount (xs:decimal) -->
      </xs:sequence>
      <!-- без транзакций балансы равны -->
      <xs:assert test="if (not(Transactions/Tx)) then AccountInfo/saldoAwal = AccountInfo/saldoAkhir else true()"/>
      <!-- начальный баланс + приход − расход = конечный -->
      <xs:assert test="if (Transactions/Tx) then (AccountInfo/saldoAwal + sum(Transactions/Tx[type='cr']/amount)
                       - sum(Transactions/Tx[type='dr']/amount) = AccountInfo/saldoAkhir) else true()"/>
    </xs:complexType>
  </xs:element>

  <!-- фасет: неотрицательная сумма не больше чем с двумя знаками после точки -->
  <xs:simpleType name="Amount">
    <xs:restriction base="xs:decimal">
      <xs:assertion test="$value ge 0 and $value * 100 = floor($value * 100)"/>
    </xs:restriction>
  </xs:simpleType>
</xs:schema>
```

```
строка 2, столбец 1: /Statement: не выполнено утверждение if (Transactions/Tx) then (AccountInfo/saldoAwal + …
/payment/amount: значение "10.005" не удовлетворяет утверждению $value ge 0 and $value * 100 = floor($value * 100)
```

- **Точная десятичная арифметика.** Значения типа `xs:decimal` и производных (`xs:integer`, `xs:long` …) и числа в самом выражении — точные десятичные числа любой длины: `+ - *`, `div` (18 знаков после точки), `mod`, `sum()`, `floor`/`ceiling`/`round` и сравнения считаются без `double`. Поэтому `0.10 + 0.20 = 0.30` истинно, десять приходов по `0.10` дают ровно `1.00`, а суммы за пределами точности `double` сходятся до копейки. Нетипизированный текст, похожий на десятичное число, тоже складывается точно; `xs:double`/`xs:float` считаются в `double`, как требует XPath 2.0.
- **Типизированные значения.** Узел атомизируется по своему типу из проверки (аналог PSVI): `xs:decimal` — число, `xs:boolean` — логическое значение, `xs:date`/`xs:dateTime` — моменты времени с учётом часового пояса, остальное — строки. Поэтому `1000.00 = 1000` истинно. Пустой элемент со значением `default` видит это значение. У элемента с содержимым только из элементов типизированного значения нет: сравнение с ним — ошибка.
- **Язык утверждений** — весь XPath 1.0 и частые конструкции XPath 2.0: `if (…) then … else …`, `some`/`every $x in … satisfies …` (несколько привязок через запятую), сравнения значений `eq ne lt le gt ge`, конструкторы `xs:string`, `xs:boolean`, `xs:decimal`, `xs:integer`, `xs:double`, `xs:date`, `xs:dateTime`, функции `exists()` и `empty()`. Имя без префикса — вне пространств имён, если не задан `xpathDefaultNamespace` (на `xs:assert`, `xs:assertion` или `xs:schema`: URI, `##targetNamespace`, `##defaultNamespace`, `##local`).
- **Всё прочее из XPath 2.0 — ошибка загрузки схемы**, а не «тихая» истина при проверке: `for`, `instance of`, `cast as`, `castable as`, `to`, `idiv`, `intersect`/`except`, `is`, последовательности `(1, 2)`, числа с экспонентой, функции вне списка (`matches`, `tokenize`, `string-join`, `fn:*` …), конструкторы других типов (`xs:time`, длительности) и абсолютные пути (корень дерева утверждения — сам элемент, а не документ). Сообщение указывает строку схемы и текст утверждения: `схема, строка 12: утверждение test="…" некорректно или использует неподдерживаемую конструкцию XPath 2.0: …`.
- **Ошибка вычисления — нарушение**, как требует XSD 1.1: строку нельзя привести к числу, деление на ноль, арифметика над датами, несколько узлов там, где ожидается одно значение, — утверждение не выполнено, причина — в сообщении.
- **Наследование.** Утверждения базового типа проверяются и у производных (`extension`, `restriction`, `xsi:type`); фасеты `xs:assertion` накапливаются по шагам ограничения, а у объединения `$value` типизирован подошедшим членом.
- **Три режима — одни и те же нарушения** (путь, строка, столбец): проверка готового дерева, разбор в DOM со схемой и потоковый `SaxParser`. Утверждению нужен весь элемент, поэтому поток копирует в мини-дерево поддерево только тех элементов, у типа которых есть утверждения, и вычисляет их при закрытии элемента; остальная проверка по-прежнему занимает O(глубины). Мини-дерево вложенного элемента после проверки переходит во внешнее без копирования, обход — по явному стеку.

### Ограничения идентичности: xs:key, xs:unique, xs:keyref

Реляционные проверки внутри документа выполняются силами самой схемы, без кода на стороне приложения:

```xml
<xs:element name="shop">
    <xs:complexType>...</xs:complexType>
    <!-- артикул товара обязателен и уникален в магазине -->
    <xs:key name="productKey">
        <xs:selector xpath="s:product"/>
        <xs:field xpath="s:sku"/>
    </xs:key>
    <!-- штрихкод необязателен, но если есть — уникален -->
    <xs:unique name="eanUnique">
        <xs:selector xpath="s:product"/>
        <xs:field xpath="@ean"/>
    </xs:unique>
    <!-- каждый заказ ссылается на существующий товар -->
    <xs:keyref name="orderRef" refer="s:productKey">
        <xs:selector xpath="s:order"/>
        <xs:field xpath="@sku"/>
    </xs:keyref>
</xs:element>
```

```
/shop/product[3]: значение (1) повторяется: нарушено ограничение xs:key "productKey"
/shop/order[2]: keyref "orderRef": значение (7) не найдено среди значений ключа "productKey"
```

- **Сравнение по значению типа**, а не по строке: для `xs:decimal` значения `1`, `1.0` и `01` — один ключ, для `xs:boolean` совпадают `1` и `true`, даты сравниваются с учётом часового пояса. Тип поля берётся из проверки документа: тип атрибута или простой тип элемента. Канонический вид значения доступен и напрямую: `beresta::xsd::canonicalValue(type, value)`.
- **Составные ключи** из нескольких `xs:field`, поля-атрибуты и поля-элементы, селекторы на любой глубине (`.//item`) и объединения (`a | b`).
- **Правила спецификации:** у `xs:key` каждое поле обязано быть, у `xs:unique` строки без значений пропускаются, поле не может выбирать больше одного узла или элемент со сложным содержимым.
- **Области видимости:** таблица ключей строится для каждого элемента с объявленным ограничением, а `xs:keyref` находит ключ, объявленный на том же элементе или на его потомках.
- **Ошибки схемы ловятся при загрузке:** необъявленный `refer`, ссылка `keyref` на другой `keyref`, разное число полей у `keyref` и ключа, повтор имени ограничения, синтаксическая ошибка в `xpath`.
- Префиксы в `xpath` разрешаются по объявлениям `xmlns` документа схемы. Имя без префикса, как и положено в XSD 1.0, — вне пространств имён.
- Селекторы и поля компилирует тот же движок XPath 1.0 один раз при загрузке схемы. Он принимает и выражения сверх грамматики XSD, которую реальные схемы и так соблюдают.
- **Потоково (SAX без дерева)** ключи проверяются, как в Xerces: селекторы и поля из грамматики XSD (оси `child` и `attribute`, `.//` в начале, `|`, `*`, `p:*`) сопоставляются со стеком имён открытых элементов, строка ключа собирается к закрытию целевого элемента, таблицы и ссылки — к закрытию области. Сообщения те же, что и при проверке по дереву. Ограничение с выражением вне этой грамматики (например, `a//b` или с предикатом) без дерева не проверить — об этом приходит одно сообщение на ограничение.

Простые типы доступны и без схемы:

```cpp
const beresta::xsd::SimpleType* type = beresta::xsd::builtinType("dateTime");
std::optional<std::string> error = beresta::xsd::validateSimpleValue(*type, "2026-10-17T12:00:00+03:00", nullptr);
bool equal = beresta::xsd::valuesEqual(*beresta::xsd::builtinType("decimal"), "1.0", "001");   // true
```

---

## XPath 1.0

```cpp
// Быстрые обёртки
std::vector<beresta::Element*> books = beresta::xpath::selectElements(doc, "//book[price > 300]");
std::string title = beresta::xpath::evaluateString(doc, "string(//book[1]/title)");

// Скомпилированное выражение: многократно и из разных потоков
beresta::xpath::XPathExpression expression("//s:item[@code = $code]/s:name");
beresta::xpath::XPathContext context;
context.namespaces.emplace("s", "urn:shop");
context.variables.emplace("code", beresta::xpath::XPathValue::fromString("B2"));

beresta::xpath::XPathValue value = expression.evaluate(doc, context);
value.getType();          // NODE_SET, BOOLEAN, NUMBER, STRING
value.getNodes();         // XPathItem{node, attribute, owner, isNamespace} в порядке документа
value.toString(); value.toNumber(); value.toBoolean();
expression.selectElements(doc, context);
```

- **Оси (все 13):** `child`, `descendant`, `descendant-or-self`, `parent`, `ancestor`, `ancestor-or-self`, `following-sibling`, `preceding-sibling`, `following`, `preceding`, `attribute`, `namespace`, `self`, а также сокращения `//`, `.`, `..`, `@`.
- **Ось `namespace`** возвращает по узлу на каждый префикс, действующий на элементе: объявления самого элемента и его предков (ближайшее скрывает дальние), отмена `xmlns=""` и неявный префикс `xml`. Имя узла — префикс, строковое значение — URI, `namespace-uri()` пуст, родитель — элемент. В порядке документа узлы пространств имён идут после элемента и до его атрибутов. В `XPathItem` у такого узла `isNamespace == true`, `attribute` указывает на объявление `xmlns:p`, `owner` — на элемент. Объявления `xmlns`, как и требует XPath, атрибутами не считаются.
- **Проверки узлов:** имена, `*`, `p:*`, `node()`, `text()`, `comment()`, `processing-instruction('t')`.
- **Выражения:** предикаты (числовые = позиция), `|`, `or`, `and`, `=`, `!=`, `<`, `<=`, `>`, `>=`, `+`, `-`, `*`, `div`, `mod`, унарный минус. Сравнения множеств работают по правилам спецификации.
- **Функции (все 27):** `last`, `position`, `count`, `id`, `local-name`, `namespace-uri`, `name`, `string`, `concat`, `starts-with`, `contains`, `substring-before`, `substring-after`, `substring`, `string-length`, `normalize-space`, `translate`, `boolean`, `not`, `true`, `false`, `lang`, `number`, `sum`, `floor`, `ceiling`, `round`.
- Строковые функции считают **символы Unicode**, а не байты. Числа выводятся по правилам XPath: `0.1 + 0.2` = `0.30000000000000004`, `1 div 0` = `Infinity`.
- Как и в XPath 1.0, имя **без префикса** означает «вне пространств имён». Для документа с `xmlns="..."` свяжите префикс в `XPathContext`.

---

## Обход дерева: TreeWalker и NodeIterator

```cpp
class SkipDrafts : public beresta::NodeFilter
{
public:
    beresta::FilterResult acceptNode(const beresta::Node& node) const override
    {
        const auto& element = static_cast<const beresta::Element&>(node);
        if (element.getAttribute("draft") == "yes")
            return beresta::FilterResult::REJECT;     // скрыть вместе с поддеревом
        return beresta::FilterResult::ACCEPT;          // SKIP — скрыть узел, но показать детей
    }
};

SkipDrafts filter;
beresta::TreeWalker walker(*doc.getRoot(), beresta::show::ELEMENTS, &filter);
for (beresta::Node* n = walker.nextNode(); n != nullptr; n = walker.nextNode()) { /* ... */ }
walker.parentNode(); walker.firstChild(); walker.lastChild();
walker.nextSibling(); walker.previousSibling(); walker.previousNode();

beresta::NodeIterator texts(doc, beresta::show::TEXTS);
for (beresta::Node* t = texts.nextNode(); t != nullptr; t = texts.nextNode()) { /* ... */ }
```

Маски `show::`: `ALL_NODES`, `DOCUMENTS`, `DOCUMENT_TYPES`, `ELEMENTS`, `TEXTS`, `CDATA_SECTIONS`, `COMMENTS`, `PROCESSING_INSTRUCTIONS` (комбинируются через `|`). Алгоритмы соответствуют DOM Level 2 Traversal, включая различие `SKIP` и `REJECT`.

---

## XInclude

```xml
<book xmlns:xi="http://www.w3.org/2001/XInclude">
  <xi:include href="chapter1.xml"/>
  <xi:include href="licence.txt" parse="text" encoding="windows-1251"/>
  <xi:include href="parts.xml" xpointer="element(/1/2)"/>
  <xi:include href="maybe.xml"><xi:fallback><p>Раздел недоступен</p></xi:fallback></xi:include>
</book>
```

```cpp
beresta::FileResourceResolver resolver("docs/");
beresta::ParserOptions options;
options.resolver = &resolver;
options.processXInclude = true;
beresta::Document book = beresta::XmlParser(options).parseFile("docs/book.xml");
// или вручную для готового дерева:
std::size_t replaced = beresta::xinclude::processXIncludes(doc, options);
```

Поддерживаются `parse="xml"` и `parse="text"` (с `encoding`), вложенные включения (циклы обнаруживаются), `xi:fallback`, XPointer `element(/1/2)`, `element(id/1)` и сокращённая форма по ID.

---

## Внешние ресурсы и безопасность

По умолчанию **ничего внешнего не загружается**: ни внешний DTD, ни внешние сущности, ни XInclude, ни `xs:include`. Это закрывает XXE и SSRF. Чтобы разрешить загрузку, передайте резолвер:

```cpp
beresta::FileResourceResolver resolver("C:/data/xml");   // только локальные файлы внутри корня
options.resolver = &resolver;
options.loadExternalDtd = true;
```

`FileResourceResolver` разрешает относительные пути от `baseUri`, отвергает сетевые URI (`http:`, `https:`, `ftp:` и т.п.) и пути, выходящие за корень через `..`.

Свой загрузчик (кэш, архив, база данных) — это наследник `ResourceResolver` с методом `resolve(const ResourceRequest&)`. Запрос содержит `kind` (`EXTERNAL_DTD`, `EXTERNAL_ENTITY`, `XINCLUDE`, `SCHEMA`), `publicId`, `systemId` и `baseUri`.

Лимиты `maxEntityExpansions`, `maxEntityDepth`, `maxExpandedSize` и `maxDepth` защищают от «сущностных бомб» и чрезмерно глубоких документов. Превышение лимита — ошибка `LIMIT_EXCEEDED`.

От атак на сложность алгоритмов защита встроена: хеш-таблицы с ключами из документа (значения ID, имена) используют SipHash-1-3 со случайным ключом процесса (`utils::SeededHash`), а повторы атрибутов и расширенных имён ищутся за O(1) даже на теге с сотнями тысяч атрибутов. Пакетная установка атрибутов `Element::setAttributes` тоже линейна. Порядок обхода таких таблиц от запуска к запуску меняется, поэтому они используются только для поиска и никогда — для вывода: результат разбора и сериализации детерминирован.

---

## Обработка ошибок

```
std::exception
└── beresta::DomException          код + сообщение
    └── beresta::ParseException    + позиция (offset, line, column)
```

| `DomErrorCode` | Когда |
|---|---|
| `PARSE_ERROR` | синтаксис XML |
| `NAMESPACE_ERROR` | правила пространств имён |
| `ENCODING_ERROR` | неизвестная кодировка, недопустимые байты или символы |
| `VALIDATION_ERROR` | DTD или XML Schema |
| `RESOURCE_ERROR` | внешний ресурс не загружен |
| `LIMIT_EXCEEDED` | превышен защитный лимит |
| `SCHEMA_ERROR` | ошибка в самой схеме или в шаблоне `pattern` |
| `XPATH_ERROR` | синтаксис или вычисление XPath |
| `INVALID_NAME`, `INVALID_CONTENT` | недопустимое имя или содержимое узла |
| `HIERARCHY_VIOLATION`, `INDEX_OUT_OF_RANGE`, `NULL_NODE`, `NOT_FOUND` | операции с деревом |
| `IO_ERROR` | файлы и потоки |

```cpp
try { /* ... */ }
catch (const beresta::ParseException& e) { e.getLine(); e.getColumn(); e.getCode(); }
catch (const beresta::DomException& e)   { e.getCode(); e.getMessage(); beresta::errorCodeToString(e.getCode()); }
```

### ErrorHandler

| Серьёзность | Пример | Поведение |
|---|---|---|
| `WARNING` | — | по умолчанию игнорируется |
| `RECOVERABLE_ERROR` | ошибки DTD и XSD | если `error()` не бросает, разбор продолжается |
| `FATAL_ERROR` | синтаксис, кодировка | после `fatalError()` парсер всегда бросает исключение |

`CollectingErrorHandler` собирает всё в `getDiagnostics()`, есть также `hasErrors()` и `clear()`.

---

## Языки сообщений

Тексты ошибок не разбросаны по коду строковыми литералами: каждое сообщение — константа `beresta::i18n::Message` (`include/i18n/message.hpp`, 302 сообщения), а тексты лежат в каталогах языков `src/i18n/catalogs/`. В комплекте два языка: русский (`ru`) и английский (`en`).

```
[PARSE_ERROR] строка 1, столбец 7: ожидался </b>, найден </a>     ← ru
[PARSE_ERROR] line 1, column 7: expected </b>, found </a>          ← en
```

Имя кода ошибки (`PARSE_ERROR`, `VALIDATION_ERROR`...) одинаково во всех языках, поэтому программная обработка ошибок от языка не зависит.

**Язык по умолчанию** выбирается при сборке. Неизвестный код останавливает сборку с понятной ошибкой:

```bash
make ERRORS_LANG=en
```
```bash
cmake -S . -B build -DBERESTA_ERRORS_LANG=en
```

**Во время работы** язык меняется одним вызовом — сразу для всех потоков. Неизвестный код отвергается, и язык остаётся прежним:

| Язык | Сменить | Текущий | Все языки |
|---|---|---|---|
| C++ | `beresta::i18n::setLanguage("en")` (→ `bool`) | `beresta::i18n::language()` | `beresta::i18n::availableLanguages()` |
| C | `beresta_set_language("en")` (→ `BERESTA_INVALID_ARGUMENT`, если языка нет) | `beresta_language()` | `beresta_languages()` → `"ru,en"` |
| Java | `Languages.use("en")` | `Languages.current()` | `Languages.available()` |
| C# | `BerestaLibrary.Language = "en"` | `BerestaLibrary.Language` | `BerestaLibrary.Languages` |
| Python | `beresta.set_language("en")` | `beresta.language()` | `beresta.languages()` |
| Node.js, WebAssembly | `beresta.setLanguage('en')` | `beresta.language()` | `beresta.languages()` |

Собственные сообщения обёрток («документ уже закрыт» и т.п.) тоже берутся из общего каталога через `beresta_message_text`: язык, добавленный в библиотеку, обёртки подхватывают без изменений.

Шаблоны сообщений содержат метки `{0}`, `{1}`... — их порядок в переводе может быть любым; `{{0}}` выводит аргумент в фигурных скобках (так записываются имена вида `{urn:shop}item`). Если в каталоге языка нет какого-то текста, используется английский.

```cpp
std::string text = beresta::i18n::format(beresta::i18n::Message::LIMIT_DEPTH, {"1024"});
```

### Как добавить язык

1. Скопируйте `src/i18n/catalogs/messages_en.cpp` в `messages_<код>.cpp` (например, `messages_de.cpp`) и переведите тексты, сохраняя метки `{N}`.
2. В конце файла переименуйте каталог: `const Catalog GERMAN_CATALOG{"de", "Deutsch", TEXTS, ...};`.
3. Объявите его в `include/i18n/i18n.hpp` (`extern const Catalog GERMAN_CATALOG;`) и добавьте строку `&GERMAN_CATALOG` в реестр `catalogs()` в `src/i18n/i18n_registry.cpp`.

Больше ничего менять не нужно: Makefile и CMake находят каталоги сами, поэтому сразу работают `make ERRORS_LANG=de` и `-DBERESTA_ERRORS_LANG=de`, а `Languages.use("de")` и аналоги — во всех обёртках.

---

## Гарантии и потокобезопасность

- **Строгая гарантия исключений** у `appendChild`, `insertChild`, `replaceChild`, `setRoot`, `set*`-методов, `setAttributes` (имена проверяются до изменений) и копирующего присваивания: при ошибке ничего не меняется.
- **`XmlParser`, `SaxParser`, `XmlSerializer`, `XPathExpression`, `Schema`, `SchemaValidator`** неизменяемы: один объект можно одновременно использовать из многих потоков (если ваши `resolver` и `errorHandler` тоже потокобезопасны).
- **Разные деревья** можно обрабатывать параллельно без синхронизации. **Одно дерево** можно читать из многих потоков через `const`-методы. Если хотя бы один поток его меняет, нужна внешняя синхронизация.
