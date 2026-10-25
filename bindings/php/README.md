# Берёста для PHP

Обёртка над Си-интерфейсом «Берёсты» через встроенное расширение PHP `ffi`: DOM, XPath 1.0, XML Schema с `xs:key`/`xs:keyref`, DTD, восемь кодировок. PHP 8.2+, сторонних пакетов нет.

```bash
composer require ermaleyalekov/beresta
```

```php
use Beresta\Parser;

$document = (new Parser())->parseString("<shop><item code='A1'>Чай</item></shop>");
$item = $document->root()->firstChild();
echo $item['code'], ' ', $item->text();          // A1 Чай
```

Нужны расширение `ffi` (`extension=ffi` в php.ini) и бинарник библиотеки для своей платформы в `native/<os>-<arch>/` (`make PHP=1` или `cmake -DBERESTA_BUILD_PHP=ON` в корне репозитория) либо путь к нему в переменной окружения `BERESTA_LIBRARY`.

**Классы.** `Parser` разбирает строки и файлы (флаги `ParseFlags`, схема для проверки при разборе). `Document` — документ: `Document::create('order')`, `save()`, `clone`, `close()`. `Node` и `Element` — узлы с `foreach`, `count()`, `echo`, атрибутами через `$element['name']`. `XPath` компилируется один раз и вычисляется многократно. `Schema` проверяет готовое дерево (`validate`) или поток без дерева (`validateFile`, `validateString`). Все исключения пакета реализуют `BerestaThrowable`; ошибки библиотеки — `BerestaException` с `status()` и `statusName()`.

**Веб-сервер.** В PHP-FPM и Apache по умолчанию `ffi.enable=preload`: FFI разрешён только в скрипте `opcache.preload`. Загрузите библиотеку там:

```php
// preload.php (opcache.preload=/path/preload.php)
require __DIR__ . '/vendor/autoload.php';
Beresta\Beresta::preload();
```

Либо задайте `ffi.enable=1`.

**Как получить скорость C++.** Каждый шаг ручного обхода (`firstChild`, `nextSibling`) — вызов через FFI и объект PHP, поэтому пошаговый обход в 13 раз медленнее, чем у ext/dom. Тяжёлую работу отдавайте ядру целиком:

```php
use Beresta\{Schema, XPath};

$schema = Schema::fromFile('export.xsd');
$problems = $schema->validateFile('export.xml');                   // поток без дерева, хоть гигабайты
$prices = (new XPath('//item/@price'))->selectValues($document);   // строки без объектов-узлов
```

**Зачем выбирать.** Для нагруженных и защищённых систем главное — память и проверка входящих файлов по схеме. Дерево «Берёсты» на 31 % компактнее, чем у ext/dom, а потоковая проверка по XSD быстрее `XMLReader`: в той же памяти помещается больше документов, и каждый файл проверяется, не замедляя сервис.

Замеры на каталоге 13 МБ, PHP 8.4 в Docker, против ext/dom и SimpleXML (libxml2 2.9):
- потоковая проверка по XSD (`validateFile`, `validateString`) на 20 % быстрее `XMLReader` со схемой: 98 против 81 МБ/с;
- XPath и разбор с проверкой по XSD — наравне с ext/dom (63 против 64 мс, 60 против 59 МБ/с);
- дерево на 31 % компактнее: 86 против 124 МБ;
- разбор в дерево медленнее: 108 против 169 МБ/с;
- нарушения схемы приходят списком `Diagnostic` со строкой и столбцом, без `libxml_use_internal_errors`; строка и столбец у `validate($document)` — с `ParseFlags::TRACK_LOCATIONS` при разборе;
- одно и то же ядро во всех языках: те же сообщения об ошибках (по-русски или по-английски) и те же правила XPath и XSD, что у Java, .NET, Python и Node.js.

Повторить: `benchmarks/wrappers/php/bench.php` и `stream_bench.php`.

Самопроверка: `composer test` или `php -d ffi.enable=1 tests/run.php`.

---

# Beresta for PHP

A wrapper over the Beresta C interface through the built-in PHP `ffi` extension: DOM, XPath 1.0, XML Schema with `xs:key`/`xs:keyref`, DTD, eight encodings. PHP 8.2+, no third-party packages.

```bash
composer require ermaleyalekov/beresta
```

The `ffi` extension (`extension=ffi` in php.ini) and the library binary for your platform in `native/<os>-<arch>/` (`make PHP=1` or `cmake -DBERESTA_BUILD_PHP=ON` in the repository root) are needed, or the path to it in the `BERESTA_LIBRARY` environment variable.

**Classes.** `Parser` parses strings and files (the `ParseFlags` flags, a schema for validation while parsing). `Document` is a document: `Document::create('order')`, `save()`, `clone`, `close()`. `Node` and `Element` are nodes with `foreach`, `count()`, `echo`, attributes through `$element['name']`. `XPath` is compiled once and evaluated many times. `Schema` validates a ready tree (`validate`) or a stream without a tree (`validateFile`, `validateString`). All package exceptions implement `BerestaThrowable`; library errors are `BerestaException` with `status()` and `statusName()`.

**Web server.** PHP-FPM and Apache have `ffi.enable=preload` by default: FFI is allowed only in the `opcache.preload` script. Load the library there (see the `preload.php` example above), or set `ffi.enable=1`.

**How to get C++ speed.** Every step of a manual walk (`firstChild`, `nextSibling`) is a call through FFI and a PHP object, so a step-by-step walk is 13 times slower than with ext/dom. Hand heavy work over to the core entirely (see the example above): `validateFile` validates a file as a stream, even gigabytes, and `selectValues` collects the strings inside the library.

**Why choose it.** For high-load and secure systems what matters most is memory and validating incoming files against a schema. The Beresta tree is 31 % more compact than with ext/dom, and streaming XSD validation is faster than `XMLReader`: more documents fit into the same memory, and every file is validated without slowing the service down.

Measurements on a 13 MB catalog, PHP 8.4 in Docker, against ext/dom and SimpleXML (libxml2 2.9):
- streaming XSD validation (`validateFile`, `validateString`) is 20 % faster than `XMLReader` with a schema: 98 against 81 MB/s;
- XPath and parsing with XSD validation are on par with ext/dom (63 against 64 ms, 60 against 59 MB/s);
- the tree is 31 % more compact: 86 against 124 MB;
- parsing into a tree is slower: 108 against 169 MB/s;
- schema violations come as a list of `Diagnostic` with the line and column, without `libxml_use_internal_errors`; the line and column of `validate($document)` come with `ParseFlags::TRACK_LOCATIONS` at parse time;
- the same core in all languages: the same error messages (in Russian or English) and the same XPath and XSD rules as in Java, .NET, Python and Node.js.

To repeat: `benchmarks/wrappers/php/bench.php` and `stream_bench.php`.

Self-test: `composer test` or `php -d ffi.enable=1 tests/run.php`.
