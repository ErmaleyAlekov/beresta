# Берёста

**Берёста** (`beresta`) — XML-библиотека на **C++17** без единой сторонней зависимости. Она полностью заменяет Xerces-C++: DOM, SAX, пространства имён, кодировки, DTD, XML Schema, XPath 1.0, XInclude, обход дерева и сериализация.

Ядро собирается в динамическую библиотеку с плоским Си-интерфейсом. Поэтому «Берёстой» можно пользоваться не только из C++, но и из **C**, **Java 22+** (Project Panama, без JNI), **C#** (P/Invoke) и **Python** (ctypes). Готовые обёртки лежат в `bindings/`.

Название — от берестяных грамот: на бересте на Руси писали документы. Библиотека тоже работает с документами, только с XML.

```cpp
#include "beresta.hpp"

beresta::Document doc = beresta::XmlParser().parse("<greeting lang=\"ru\">Привет</greeting>");
std::cout << doc.getRoot()->getTextContent();                    // Привет
std::cout << beresta::xpath::evaluateString(doc, "/*/@lang");     // ru
std::cout << doc.toString();                                      // <greeting lang="ru">Привет</greeting>
```

---

## Содержание

1. [Преимущества](#преимущества)
2. [Сравнение с Xerces-C++](#сравнение-с-xerces-c)
3. [Сборка и подключение](#сборка-и-подключение)
4. [Быстрый старт](#быстрый-старт)
5. [Модель данных и память](#модель-данных-и-память)
6. [Работа с деревом](#работа-с-деревом)
7. [Копирование и перенос узлов](#копирование-и-перенос-узлов)
8. [Разбор: DOM](#разбор-dom)
9. [Разбор: SAX](#разбор-sax)
10. [Пространства имён](#пространства-имён)
11. [Кодировки, файлы и потоки](#кодировки-файлы-и-потоки)
12. [Сериализация](#сериализация)
13. [DTD и сущности](#dtd-и-сущности)
14. [XML Schema](#xml-schema)
15. [XPath 1.0](#xpath-10)
16. [Обход дерева: TreeWalker и NodeIterator](#обход-дерева-treewalker-и-nodeiterator)
17. [XInclude](#xinclude)
18. [Внешние ресурсы и безопасность](#внешние-ресурсы-и-безопасность)
19. [Обработка ошибок](#обработка-ошибок)
20. [Гарантии и потокобезопасность](#гарантии-и-потокобезопасность)
21. [Другие языки: Си-интерфейс](#другие-языки-си-интерфейс)
22. [Java (Project Panama)](#java-project-panama)
23. [C# (P/Invoke)](#c-pinvoke)
24. [Python (ctypes)](#python-ctypes)
25. [Ограничения](#ограничения)
26. [Структура проекта](#структура-проекта)
27. [Тесты](#тесты)
28. [Расширение библиотеки](#расширение-библиотеки)
29. [Лицензия](#лицензия)

---

## Преимущества

### Современный C++ вместо наследия Xerces
- **Только `std::string` в UTF-8.** Нет `XMLCh`, `XMLString::transcode()` и ручного `release()`. Значения из дерева сразу подходят любому коду на C++.
- **RAII везде.** Родитель владеет детьми через `std::unique_ptr`, поэтому `delete`, `release()` и `XMLPlatformUtils::Initialize/Terminate` не нужны, а утечки невозможны по построению.
- **Узлы автономны.** Глобального менеджера документа нет. Узел создаётся обычным конструктором, живёт где угодно и переносится в любое дерево без `importNode`/`adoptNode`.
- **Глубокое копирование одной строкой:** `Element copy = original;`. Для копирования через базовый класс есть виртуальный `clone()`.
- **Типизированные настройки.** Вместо строковых «фич» `DOMConfiguration` — обычная структура `ParserOptions`, и опечатку в имени ловит компилятор.
- **Строгие гарантии исключений.** Если вставка, замена или присваивание не удались, дерево остаётся прежним, а переданный узел остаётся у вызывающего.

### Разбор
- **Один парсер — два режима.** SAX-ядро генерирует события, DOM строится одним из обработчиков (паттерн Builder). SAX так же строг, как DOM.
- **Формат ввода не важен.** Отформатированный и «сплошной» XML дают одно и то же дерево.
- **Разбор на `std::string_view`.** Для UTF-8 входной текст не копируется: сканер возвращает срезы исходной строки.
- **Итеративный разбор без рекурсии.** Открытые элементы и тексты сущностей хранятся в явных стеках, поэтому глубокий ввод не переполнит стек вызовов.
- **Позиции ошибок в символах UTF-8.** Координаты считаются инкрементально и только по запросу. Позицию любого узла можно запомнить (`trackLocations`).

### Полный набор стандартов
- **Namespaces in XML:** проверка при разборе и «живые» URI в DOM.
- **Восемь кодировок:** UTF-8, UTF-16 LE/BE, ISO-8859-1, US-ASCII, windows-1251, windows-1252, KOI8-R, IBM866. Кодировка определяется по BOM и декларации. Все таблицы написаны самостоятельно, без iconv и ICU.
- **DTD:** внутреннее и внешнее подмножества, параметрические сущности, условные секции, атрибуты по умолчанию, полная проверка валидности, `getElementById`.
- **XML Schema 1.0:** 45 встроенных типов, фасеты, `restriction`/`extension`, `list`/`union`, `sequence`/`choice`/`all`, группы, `xs:any`, `xsi:type`, `xsi:nil`, ID/IDREF, `include`/`import`.
- **XPath 1.0:** все оси, кроме `namespace`, все 27 функций базовой библиотеки, переменные и префиксы.
- **XInclude 1.0:** `parse="xml|text"`, `xi:fallback`, XPointer `element()` и по ID, защита от циклов.
- **DOM Traversal:** `TreeWalker` и `NodeIterator` с фильтрами — алгоритмы строго по спецификации W3C.

### Надёжность и безопасность
- **Защита от «billion laughs».** Лимиты на число, объём и вложенность раскрытий сущностей включены всегда, а не отдельной настройкой.
- **Нет XXE и SSRF по умолчанию.** Без явного `ResourceResolver` ничего внешнего не загружается. `FileResourceResolver` не выходит за корневой каталог и отвергает сетевые URI.
- **Своё регулярное выражение XSD на НКА Томпсона.** Время проверки линейно, «катастрофический бэктрекинг» невозможен в принципе.
- **Точная арифметика фасетов.** Десятичные числа сравниваются как строки цифр любой длины, без потери точности на `double`.
- **Проверено санитайзерами:** все тесты проходят под AddressSanitizer, LeakSanitizer, UBSan и `_GLIBCXX_ASSERTIONS` — без утечек памяти, неопределённого поведения и выхода за границы контейнеров.

### Вывод
- **Компактный по умолчанию.** Ни одного лишнего пробела, порядок атрибутов сохраняется, повторный разбор вывода даёт тот же текст.
- **Красивый вывод по запросу.** Отступы ставятся только там, где они не меняют данные: смешанное содержимое выводится дословно.
- **Любая выходная кодировка.** Декларация сама получает правильный `encoding`, BOM пишется при необходимости, а символы вне кодировки становятся `&#N;`.

### Один бинарник — пять языков
- **Плоский Си-интерфейс** (`extern "C"`, 64 функции): стабильный ABI, имена без декорирования, объекты наружу — непрозрачные дескрипторы `void*`. Исключения C++ никогда не пересекают границу библиотеки.
- **Java 22+ через Project Panama** (Foreign Function & Memory API): ни строчки JNI, Java работает с нативной памятью напрямую. Бинарник едет внутри jar и сам извлекается под нужную ОС и архитектуру.
- **C# через P/Invoke** с `SafeHandle`: память освобождается детерминированно (`using`) и гарантированно (при финализации). Пакет NuGet со стандартной раскладкой `runtimes/<rid>/native`.
- **Python через ctypes:** ни одного пакета из PyPI и ни одного компилятора на машине пользователя.
- **Сборка сама раскладывает бинарник.** CMake и Makefile определяют ОС и архитектуру процессора и копируют библиотеку в ресурсы Maven-проекта, в `runtimes/<rid>/native` для .NET и в пакет Python.
- **Наружу видны только функции Си-интерфейса** — ровно 64 символа. Классы C++ скрыты (`-fvisibility=hidden`, на Windows экспортируется только `BERESTA_API`). C++ runtime встроен в библиотеку, поэтому на чистой машине не нужны ни `libstdc++`, ни `vcruntime`. На Linux символы встроенной `libstdc++` тоже спрятаны (`--exclude-libs,ALL`): в процессе JVM или Python они не конфликтуют с системной `libstdc++`.

### Инженерные качества
- **Ноль зависимостей:** только стандартная библиотека C++17. Обёрткам тоже ничего не нужно, кроме JDK, .NET или Python.
- **Кроссплатформенность:** Windows, Linux, macOS. Компиляторы MSVC, MinGW, GCC, Clang. Есть CMake и Makefile с автозависимостями.
- **Тип узла без `dynamic_cast` в ядре:** тип определяется через `getType()`, приведение — `static_cast`. Только Си-интерфейс проверяет дескрипторы через `dynamic_cast`: текстовый узел вместо элемента даёт ошибку `INVALID_ARGUMENT`, а не неопределённое поведение.
- **Читаемый код:** функции не длиннее 25 строк, в файле не больше 5 функций, подробные комментарии на русском, классические паттерны (Composite, Template Method, Builder, Facade, Strategy, Interpreter, Observer, Factory Method).

---

## Сравнение с Xerces-C++

| Возможность | Xerces-C++ | Берёста |
|---|---|---|
| Строки | `XMLCh*` (UTF-16), `transcode`/`release` | `std::string` (UTF-8) |
| Инициализация | `XMLPlatformUtils::Initialize()` | не нужна |
| Создание узла | только `DOMDocument::createElement` | `std::make_unique<beresta::Element>("a")` |
| Узел вне документа | привязан к документу-владельцу | полностью автономен |
| Перенос между документами | `importNode` / `adoptNode` | `removeChild` + `appendChild` |
| Освобождение памяти | `release()` / удаление документа | RAII (`unique_ptr`) |
| Настройки | строковые параметры `DOMConfiguration` | структура `ParserOptions` |
| DOM, SAX | есть | есть (`XmlParser`, `SaxParser`) |
| Пространства имён | есть | есть, URI в DOM вычисляются «вживую» |
| Кодировки | через транскодеры, ICU или iconv | 8 кодировок, собственные таблицы |
| DTD-валидация | есть | есть |
| XML Schema | есть (полная) | есть (подмножество, см. [Ограничения](#ограничения)) |
| XPath | только для ограничений идентичности XSD | полный XPath 1.0 |
| XInclude | есть | есть |
| TreeWalker / NodeIterator | есть | есть |
| Защита от сущностных бомб | `SecurityManager` по отдельной настройке | всегда включена |
| Внешние ресурсы | загружаются по умолчанию | только через явный `ResourceResolver` |
| Форматированный вывод | `format-pretty-print` | `SerializerOptions::prettyPrint` |
| Другие языки | только C++ (обёртки — сторонние проекты) | C, Java 22+, C#, Python в комплекте |
| Зависимости | библиотека + транскодеры | нет |

---

## Сборка и подключение

Нужен компилятор с поддержкой **C++17**. Есть два независимых способа сборки — CMake и Makefile; оба работают на Windows, Linux и macOS.

Что получается в результате:

| Файл | Для кого |
|---|---|
| `beresta.dll` / `libberesta.so` / `libberesta.dylib` | динамическая библиотека с Си-интерфейсом: C, Java, C#, Python |
| `libberesta.a` (Makefile) / `beresta_static` (CMake) | статическая библиотека для программ на C++ (полный C++ API) |

Из динамической библиотеки экспортируются **только** функции Си-интерфейса. Программы на C++ подключают статическую библиотеку.

### Makefile (GCC / Clang / MinGW)

```bash
make
```

| Цель | Что делает |
|---|---|
| `make` / `make all` | `bin/libberesta.a`, динамическая библиотека, демо `bin/beresta_demo` (C++) и `bin/beresta_capi_demo` (C) |
| `make test` | собирает и запускает модульные тесты C++ |
| `make JAVA=1` | то же + бинарник в ресурсы Maven-проекта |
| `make CSHARP=1` | то же + бинарник в `runtimes/<rid>/native` проекта .NET |
| `make PYTHON=1` | то же + бинарник в пакет Python |
| `make bindings-test` | самопроверки включённых обёрток, например `make JAVA=1 CSHARP=1 PYTHON=1 bindings-test` |
| `make java-test`, `csharp-test`, `python-test` | самопроверка одной обёртки (копирует бинарник сама) |
| `make java-package` | jar через Maven (`mvn package`) |
| `make csharp-package` | пакет NuGet (`dotnet pack`) |
| `make platform` | показать определённые ОС, архитектуру и пути копирования |
| `make clean` | удаляет `obj/` и `dep/` |
| `make fclean` | `clean` + `bin/`, копии бинарника в обёртках и `bindings/java/target/` |
| `make re` | полная пересборка |

Особенности Makefile:
- `make -j` поддерживается, компилятор можно сменить (`make CXX=clang++ CC=clang`).
- Объектные и `.d`-файлы раскладываются по `obj/` и `dep/` зеркально `src/`. Изменённый заголовок пересобирает только зависимые файлы, а изменённый Makefile (например, флаги) — все объекты.
- Очистка сама определяет оболочку: `rmdir /S /Q` в `cmd.exe` на Windows и `rm -rf` в `sh` на Linux и macOS.
- Архитектура берётся у самого компилятора (`$(CXX) -dumpmachine`), поэтому при кросс-компиляции бинарник попадает в папку целевой платформы.

### CMake (MSVC / MinGW / GCC / Clang)

По умолчанию CMake собирает **только ядро** — динамическую библиотеку:

```bash
cmake -S . -B build
```
```bash
cmake --build build --config Release
```

Всё остальное включается флагами:

| Опция | По умолчанию | Что делает |
|---|---|---|
| `BERESTA_BUILD_STATIC` | `OFF` | статическая библиотека `beresta_static` (`beresta::static`) для C++ |
| `BERESTA_BUILD_EXAMPLES` | `OFF` | демо на C++ и на чистом C |
| `BERESTA_BUILD_TESTS` | `OFF` | модульные тесты и самопроверки включённых обёрток в `ctest` |
| `BERESTA_BUILD_JAVA` | `OFF` | после сборки бинарник копируется в ресурсы Maven-проекта; цель `beresta_java_package` вызывает `mvn package` |
| `BERESTA_BUILD_CSHARP` | `OFF` | бинарник в `runtimes/<rid>/native`; цель `beresta_csharp_package` вызывает `dotnet pack` |
| `BERESTA_BUILD_PYTHON` | `OFF` | бинарник в пакет Python |
| `BERESTA_STATIC_RUNTIME` | `ON` | встроить C++ runtime в библиотеку (`/MT` для MSVC, `-static` для MinGW, `-static-libstdc++` для GCC) |

Полная сборка со всеми обёртками и тестами:

```bash
cmake -S . -B build -DBERESTA_BUILD_TESTS=ON -DBERESTA_BUILD_JAVA=ON -DBERESTA_BUILD_CSHARP=ON -DBERESTA_BUILD_PYTHON=ON
```
```bash
cmake --build build --config Release
```
```bash
ctest --test-dir build -C Release --output-on-failure
```

Для MSVC автоматически включаются `/utf-8 /W4 /permissive-`.

### Куда копируется бинарник

CMake (модуль `cmake/BerestaPlatform.cmake`) и Makefile определяют **целевую** ОС и архитектуру и приводят имена к единому виду. Те же имена ищут загрузчики обёрток:

| Обёртка | Папка |
|---|---|
| Java | `bindings/java/src/main/resources/natives/<os>-<arch>/` |
| C# | `bindings/csharp/Beresta/runtimes/<rid>/native/` |
| Python | `bindings/python/beresta/_native/<os>-<arch>/` |

- `<os>-<arch>`: `windows-x86_64`, `windows-aarch64`, `linux-x86_64`, `linux-aarch64`, `macos-x86_64`, `macos-aarch64`, `*-x86`.
- `<rid>` (идентификатор среды .NET): `win-x64`, `win-arm64`, `linux-x64`, `linux-arm64`, `osx-x64`, `osx-arm64`.
- Источник архитектуры в CMake: `MSVC_CXX_ARCHITECTURE_ID` для MSVC (верно и для `-A ARM64`), `CMAKE_OSX_ARCHITECTURES` на macOS (только одна архитектура: каждая кладётся в свою папку), иначе `CMAKE_SYSTEM_PROCESSOR`. Значения можно задать вручную: `-DBERESTA_PLATFORM_ARCH=aarch64`.
- Один jar или пакет NuGet для всех платформ: соберите ядро на каждой ОС (например, в матрице CI). Каждая сборка добавит свою папку, после чего выполните `mvn package` или `dotnet pack`.

### Подключение к своему проекту на C++

```cmake
set(BERESTA_BUILD_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/beresta)
target_link_libraries(my_app PRIVATE beresta::static)
```

Цель `beresta::beresta` — динамическая библиотека. Её подключают программы на C (через `capi/beresta_c.h`).

Или вручную:

```bash
g++ -std=c++17 -Ipath/to/beresta/include main.cpp path/to/beresta/bin/libberesta.a -o app
```

В коде достаточно одного заголовка: `#include "beresta.hpp"`.

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
| `schema` | `nullptr` | XML Schema для проверки после разбора |

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

Все `string_view` в событиях действительны **только на время вызова**. `DomBuilder` — открытый обработчик, строящий DOM; его можно унаследовать.

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
3. Атрибут `encoding` в декларации.
4. Иначе UTF-8.

`ParserOptions::forcedEncoding` перекрывает автоопределение. Байты, недопустимые в кодировке, и символы, запрещённые в XML (управляющие коды и т.п.), дают ошибку `ENCODING_ERROR` с позицией.

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
- **Проверка валидности** (`validateDtd = true`) проверяет:
  - корень и объявленность элементов и атрибутов;
  - `#REQUIRED` и `#FIXED`;
  - типы `ID`, `IDREF(S)`, `NMTOKEN(S)`, `ENTITY(IES)`, перечисления, уникальность ID и цели IDREF;
  - модели `EMPTY`, `ANY`, `MIXED` и `CHILDREN`.

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

// Вариант 2: проверка прямо при разборе
beresta::ParserOptions options;
options.schema = schema;
options.errorHandler = &handler;          // без него — исключение на первой ошибке
beresta::XmlParser(options).parseFile("catalog.xml");
```

Каждая ошибка содержит путь к элементу и позицию: `строка 12, столбец 5: /catalog/book[2]/price: значение "-5" меньше минимума 0`. При проверке схемой при разборе позиции записываются автоматически.

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
- `simpleType` `restriction`/`list`/`union`;
- фасеты `length`, `minLength`, `maxLength`, `pattern`, `enumeration`, `whiteSpace`, `minInclusive`, `maxInclusive`, `minExclusive`, `maxExclusive`, `totalDigits`, `fractionDigits`;
- все **45 встроенных типов** XSD 1.0 (строковые, целые с точными границами, `decimal`, `float`/`double`, даты и время с часовыми поясами, `duration`, `hexBinary`, `base64Binary`, `QName`, ID/IDREF(S), NMTOKEN(S), ENTITY(IES) и т.д.);
- уникальность `ID` и существование целей `IDREF`;
- `xs:include` (включая «хамелеонные» схемы) и `xs:import`.

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
value.getNodes();         // XPathItem{node, attribute, owner} в порядке документа
value.toString(); value.toNumber(); value.toBoolean();
expression.selectElements(doc, context);
```

- **Оси:** `child`, `descendant`, `descendant-or-self`, `parent`, `ancestor`, `ancestor-or-self`, `following-sibling`, `preceding-sibling`, `following`, `preceding`, `attribute`, `self`, а также сокращения `//`, `.`, `..`, `@`.
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

## Гарантии и потокобезопасность

- **Строгая гарантия исключений** у `appendChild`, `insertChild`, `replaceChild`, `setRoot`, `set*`-методов и копирующего присваивания: при ошибке ничего не меняется.
- **`XmlParser`, `SaxParser`, `XmlSerializer`, `XPathExpression`, `Schema`, `SchemaValidator`** неизменяемы: один объект можно одновременно использовать из многих потоков (если ваши `resolver` и `errorHandler` тоже потокобезопасны).
- **Разные деревья** можно обрабатывать параллельно без синхронизации. **Одно дерево** можно читать из многих потоков через `const`-методы. Если хотя бы один поток его меняет, нужна внешняя синхронизация.

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

### Соглашения (одинаковы для всех 64 функций)

| Правило | Пояснение |
|---|---|
| Дескрипторы | `beresta_document`, `beresta_node`, `beresta_xpath`, `beresta_nodeset`, `beresta_schema`, `beresta_diagnostics` — это `void*` |
| Статус | функции, возвращающие `int`, отдают `BERESTA_OK` (0) при успехе или код ошибки |
| NULL | функции, возвращающие дескриптор или `char*`, отдают NULL при ошибке. NULL с кодом 0 означает «объекта нет» (например, нет следующего соседа) |
| Последняя ошибка | `beresta_last_error_code()` и `beresta_last_error_message()` — своя у каждого потока, как `errno`. Каждый вызов её сбрасывает |
| Строки | всегда UTF-8. `char*` от библиотеки освобождается через `beresta_string_free`, `const char*` — заимствованная |
| Размеры и индексы | `int64_t` — одинаковый тип во всех языках; `-1` в результате означает ошибку |
| Владение | документ, схему, выражение, набор узлов и список диагностик освобождает вызывающий (`beresta_*_free`). Узел принадлежит документу |
| Потоки | разные документы — в разных потоках параллельно; схема после загрузки потокобезопасна |

| Группа | Функции |
|---|---|
| Библиотека | `version`, `last_error_code`, `last_error_message`, `status_name`, `string_free` |
| Документы | `document_create`, `document_parse` (байты в любой кодировке), `document_parse_file`, `document_clone`, `document_root`, `document_element_by_id`, `document_save` (любая выходная кодировка), `document_free` |
| Узлы | `node_type`, `node_name`, `node_text`, `node_to_string`, `node_parent`, `node_first_child`, `node_last_child`, `node_next_sibling`, `node_previous_sibling`, `node_owner_document`, `node_child_count`, `node_child` |
| Изменение дерева | `node_set_text`, `node_remove`, `node_copy_to` (в том числе между документами), `node_append_element`, `node_append_text`, `node_append_comment`, `node_append_cdata`, `node_append_pi` |
| Атрибуты | `element_get_attribute`, `element_set_attribute`, `element_has_attribute`, `element_remove_attribute`, `element_attribute_count`, `element_attribute_name`, `element_attribute_value`, `element_local_name`, `element_namespace_uri` |
| XPath | `xpath_compile`, `xpath_bind_namespace`, `xpath_set_string`, `xpath_set_number`, `xpath_select`, `xpath_string`, `xpath_number`, `xpath_boolean`, `nodeset_size`, `nodeset_node`, `nodeset_value`, `xpath_free`, `nodeset_free` |
| XML Schema | `schema_load`, `schema_load_file`, `schema_validate`, `diagnostics_count`, `diagnostics_message`, `diagnostics_line`, `diagnostics_column`, `schema_free`, `diagnostics_free` |

Флаги разбора — битовая маска `BERESTA_PARSE_*`: `PRESERVE_WHITESPACE`, `DISCARD_COMMENTS`, `COALESCE_CDATA`, `VALIDATE_DTD`, `NO_NAMESPACES`, `XINCLUDE`, `FILE_RESOURCES`. Последний разрешает внешние DTD, сущности и XInclude, но только из каталога разбираемого файла. Флаги вывода — `BERESTA_WRITE_PRETTY` и `BERESTA_WRITE_DECLARATION`. Маска выбрана вместо структуры настроек сознательно: целое число одинаково передаётся из любого языка.

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
     XmlDocument document = Xml.parseFile(Path.of("order.xml"), ParseFlags.DEFAULT, null))
{
    for (Diagnostic problem : schema.validate(document))
        System.out.println(problem.line() + ": " + problem.message());
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
java --enable-native-access=ALL-UNNAMED -cp beresta-2.1.0.jar:app.jar com.example.App
```

Как это устроено:
- `NativeLoader` определяет платформу по `os.name` и `os.arch`, извлекает `natives/<os>-<arch>/<файл>` из jar во временный каталог и загружает его через `SymbolLookup.libraryLookup`. Путь можно задать явно: `-Dberesta.library.path=/path/libberesta.so`.
- Дескрипторы вызовов (`MethodHandle`) строятся один раз по `FunctionDescriptor`, строки передаются через `Arena.ofConfined()` и освобождаются сразу после вызова.
- `XmlDocument`, `XPath`, `XmlSchema` реализуют `AutoCloseable`. Если объект забыли закрыть, нативную память освободит `Cleaner`.
- Узел хранит ссылку на свой документ. После `close()` обращение к узлу даёт `IllegalStateException`, а не падение JVM.
- Методы узла разнесены по интерфейсам (`NodeContent`, `NodeNavigation`, `NodeChildren`, `NodeEditing`, `NodeStructure`, `ElementAttributes`): можно принимать ровно ту часть API, которая нужна.

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
    using XmlDocument document = Xml.ParseFile("order.xml");
    foreach (Diagnostic problem in schema.Validate(document))
        Console.WriteLine(problem.Line + ": " + problem.Message);
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
    with beresta.parse_file("order.xml", ParseFlags.DISCARD_COMMENTS) as document:
        for problem in schema.validate(document):
            print(problem.line, problem.message)
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
- Библиотека ищется в `BERESTA_LIBRARY`, затем в `beresta/_native/<os>-<arch>/`, затем в системе.

Самопроверка на стандартном `unittest`: `make PYTHON=1 python-test`. Makefile запускает `python` на Windows и `python3` на Linux и macOS; другой интерпретатор задаётся так: `make PYTHON_EXE=/path/python3.12 python-test`.

---

## Ограничения

Осознанные ограничения (KISS / YAGNI):

- **XML Schema** — подмножество XSD 1.0:
  - нет ограничений идентичности (`xs:key`, `xs:keyref`, `xs:unique`);
  - `xs:redefine` и `xs:override` не поддерживаются;
  - `substitutionGroup` задаёт тип, но членство в группе замены при проверке модели не учитывается;
  - допустимость наследования для `xsi:type` и `restriction` не проверяется;
  - значения по умолчанию из схемы в DOM не добавляются;
  - у `duration` нет порядка для фасетов-границ;
  - категории Unicode в `pattern` (`\p{L}` и т.п.) покрывают основные письменности (латиница, кириллица, греческий, иврит, арабский, CJK), а не всю таблицу Unicode.
- **XPath:** без оси `namespace`, только XPath 1.0 (не 2.0/3.1).
- **XInclude:** атрибут `xml:base` у включённого содержимого не добавляется; XPointer — только `element()` и ID.
- **Кодировки:** восемь перечисленных выше. Остальные нужно перекодировать заранее или добавить своей таблицей.
- **Нет** XSLT, DOM Range, XML 1.1-специфики (`version="1.1"` разбирается по правилам 1.0) и сетевой загрузки.
- **Сериализация, `clone()` и деструктор рекурсивны.** Для деревьев от парсера глубину ограничивает `maxDepth`; при ручном построении деревьев глубиной в десятки тысяч уровней учитывайте размер стека.
- **Си-интерфейс и обёртки** покрывают DOM, XPath и XML Schema. SAX, `TreeWalker`, свой `ResourceResolver` и тонкие настройки `ParserOptions` (лимиты, принудительная кодировка) доступны только из C++. DTD-проверка и XInclude в других языках включаются флагами разбора.
- **Узел после удаления.** После `remove()` обёртка удалённого узла и обёртки его потомков недействительны, обращаться к ним нельзя. Документ проверяет только собственное закрытие.
- **32-битные платформы** поддерживаются ядром, но обёртки проверялись на 64-битных.

---

## Структура проекта

```
beresta/
├── CMakeLists.txt  Makefile  README.md
├── cmake/                        определение платформы, обёртки, самопроверка Java
├── include/                      только объявления
│   ├── beresta.hpp               общий заголовок C++
│   ├── capi/                     beresta_c.h — плоский Си-интерфейс; internal/ — барьер исключений
│   ├── core/                     Node, ContainerNode, Element, Document, DocumentType, Text, ...
│   ├── parser/                   XmlParser, ParserOptions, ErrorHandler, ResourceResolver, DomBuilder
│   │   └── internal/             стек входов, бюджет сущностей, пространства имён, декодеры
│   ├── sax/                      SaxParser, ContentHandler, Locator
│   ├── encoding/                 Charset, таблицы, определение кодировки
│   ├── dtd/                      модель, чтение, разбор и проверка DTD
│   ├── content/                  модель содержимого (общая для DTD и XSD)
│   ├── xsd/                      Schema, SchemaValidator, типы данных; regex/ — движок шаблонов
│   ├── xpath/                    XPathExpression, XPathValue; internal/ — лексер, парсер, AST
│   ├── traversal/                TreeWalker, NodeIterator, NodeFilter
│   ├── serializer/               XmlSerializer
│   ├── xinclude/                 XInclude
│   ├── io/                       файлы и потоки
│   ├── exception/                DomException, ParseException
│   └── utils/                    символы XML, UTF-8, экранирование, имена
├── src/                          реализация, зеркально include/ (≤ 5 функций в файле)
│   └── capi/                     Си-интерфейс: errors, handles, library, document, node, element, xpath, schema
├── bindings/
│   ├── java/                     Maven-проект: org.beresta (API), org.beresta.internal (Panama), selftest
│   ├── csharp/                   Beresta (библиотека, NuGet) и Beresta.SelfTest
│   └── python/                   пакет beresta, tests/, pyproject.toml
├── examples/                     демо на C++ (src/) и на C (capi/)
└── tests/                        модульные тесты C++ (собственный мини-фреймворк)
```

Ядро и Си-интерфейс — около **19,5 тысяч строк** (227 файлов реализации, 76 заголовков). Обёртки — ещё около 4,7 тысячи строк на Java, C# и Python.

### Устройство парсера

```
байты ──► encoding::decodeDocument ──► UTF-8 ──► ParseSession ──► ContentHandler
              (BOM, декларация,                (итеративный цикл,    ├── DomBuilder ──► Document ──► XInclude ──► XSD
               проверка символов)               стек сущностей,      └── ваш SAX-обработчик
                                                пространства имён,
                                                DTD + валидатор)
```

### Устройство обёрток

```
Java (FFM API) ─┐
C# (P/Invoke) ──┼──► beresta_c.h ──► барьер исключений ──► ядро C++ (DOM, XPath, XSD, ...)
Python (ctypes)─┤    (extern "C",     (код + сообщение
C, Go, Rust ────┘     void*-дескрипторы) в thread_local)
```

---

## Тесты

```bash
make test
```

**87 тестов C++ в 24 наборах:**
- ядро: копирование, иерархия, элементы, навигация, пространства имён, сериализация;
- парсер: форматы, содержимое, ошибки, лимиты, SAX, кодировки;
- DTD: сущности, проверка валидности, внешние ресурсы;
- сериализатор, обход дерева, XPath (пути, оси, функции), XSD (типы, схемы, расширенные возможности), XInclude;
- Си-интерфейс: коды ошибок, построение и изменение дерева, XPath, XML Schema.

Тесты проходят и в обычной сборке, и под `-fsanitize=undefined -D_GLIBCXX_ASSERTIONS`.

Самопроверки обёрток работают без тестовых фреймворков (JUnit, xUnit и pytest не нужны):

```bash
make JAVA=1 CSHARP=1 PYTHON=1 bindings-test
```

Проверяются разбор, навигация, построение, копирование между документами, ошибки, запись в другой кодировке, XPath и XML Schema: Java — 8 проверок, C# — 8, Python — 9. В CMake всё это (и тесты C++) запускает `ctest`, если включены `BERESTA_BUILD_TESTS` и флаги обёрток.

Где проверено (без единого предупреждения компилятора):

| Платформа | Компиляторы | Что проверено |
|---|---|---|
| Windows 11 x64 | MSVC (Visual Studio 2026), MinGW GCC 15 | CMake и Makefile, тесты C++, `ctest`, самопроверки Java 25 / .NET 9 / Python 3.12, `dotnet pack` |
| Ubuntu 26.04 x64 | GCC 15, Clang 21 | CMake и Makefile, тесты C++, `ctest`, самопроверки Java 25 / .NET 10 / Python 3.14, `mvn package` (запуск с бинарником из jar), `dotnet pack`, wheel в чистом venv |
| Ubuntu 26.04 x64 | GCC 15 + ASan, UBSan, LeakSanitizer | тесты C++, демо на C и C++: ни утечек памяти, ни неопределённого поведения |

macOS описана в CMake и Makefile (`.dylib`, `@rpath`, `CMAKE_OSX_ARCHITECTURES`), но не проверялась.

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

**Новая функция Си-интерфейса:**
1. Объявление с `BERESTA_API` в `include/capi/beresta_c.h` (типы — только C: `void*`, `int`, `int64_t`, `uint32_t`, `double`, `const char*`).
2. Реализация в `src/capi/<группа>/` в обрамлении `BERESTA_CAPI_TRY` … `BERESTA_CAPI_CATCH(...)`: ни одно исключение не должно выйти наружу.
3. Строка-сигнатура в каждой обёртке: `_signatures.py` (Python), `*Functions.java` (Java), `NativeMethods.*.cs` (C#). Затем метод в объектном API обёртки.

---

## Лицензия

«Берёста» распространяется под лицензией **MIT** — текст в файле [LICENSE](LICENSE).

Коротко: библиотеку можно бесплатно использовать, изменять и распространять, в том числе в закрытых и коммерческих продуктах. Единственное условие — сохранять уведомление об авторских правах и текст лицензии в копиях программы. Гарантий нет: программа предоставляется «как есть».

Лицензия уже вложена во все пакеты, поэтому при распространении ничего добавлять вручную не нужно:

| Пакет | Где лежит лицензия |
|---|---|
| jar (Maven) | `META-INF/LICENSE` и элемент `<licenses>` в `pom.xml` |
| NuGet | `LICENSE` в корне пакета и `PackageLicenseExpression` = `MIT` |
| wheel (Python) | `*.dist-info/licenses/LICENSE` и поле `license = "MIT"` в метаданных |

У `bindings/python` своя копия `LICENSE`, потому что setuptools не берёт файлы за пределами каталога пакета. Если меняете корневой `LICENSE`, обновите и её.

Сторонний код в проект не входит: ядро, обёртки и тесты написаны с нуля, поэтому других лицензий и уведомлений (NOTICE, THIRD-PARTY) нет.
