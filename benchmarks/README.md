# Замеры производительности

Программа `beresta_bench` сравнивает «Берёсту» с другими XML-библиотеками на одинаковых данных. Операции одинаковы для всех библиотек:
- разбор в DOM;
- обход всего дерева;
- сериализация без отступов;
- запрос XPath 1.0;
- разбор с проверкой по XML Schema (с построением дерева);
- потоковая проверка по XML Schema без дерева (SAX): «Берёста» — `SaxParser` со схемой, libxml2 — `xmlSchemaValidateStream`;
- память под дерево. Её меряют в отдельном процессе, чтобы библиотеки не делили кучу.

Данные генерируются детерминированно: каталог товаров (много мелких элементов с атрибутами и сущностями) и статья (длинный русский текст со смешанным содержимым). Для каждой операции берётся медиана из нескольких запусков, первый запуск считается прогревом. Таблица сверки показывает контрольные суммы обхода, размер вывода, число узлов XPath и число ошибок XSD. Если они совпадают у всех библиотек, значит, все построили одно и то же дерево.

Сравниваемые библиотеки подключаются, только если их нашёл pkg-config (RapidXML — по заголовку). Без них программа меряет одну «Берёсту». Сама «Берёста» от них не зависит: это только инструмент сравнения.

```bash
sudo apt install libxml2-dev libpugixml-dev libxerces-c-dev libtinyxml2-dev librapidxml-dev
```
```bash
make run
```
```bash
cmake -S . -B ../build-bench -DCMAKE_BUILD_TYPE=Release && cmake --build ../build-bench --target bench_run
```

Ключи: `--runs N` (повторы, по умолчанию 5), `--items N` (товары каталога, 100 000 ≈ 26 МБ), `--paragraphs N` (абзацы статьи, 25 000 ≈ 14 МБ), `--report файл.md`.

## Node.js

`node/compare.js` сравнивает обёртку Node.js с библиотеками npm: fast-xml-parser, @xmldom/xmldom (+ xpath) и libxmljs2. Они ставятся отдельно и ищутся в каталоге `BENCH_NODE_MODULES`. Если библиотека не установлена, она пропускается.

```bash
npm install --prefix /tmp/nodebench fast-xml-parser @xmldom/xmldom xpath libxmljs2
```
```bash
BENCH_NODE_MODULES=/tmp/nodebench node node/compare.js 20000
```

### Обёртки против родных библиотек языков

`wrappers/` сравнивает обёртки Java, .NET, Python и PHP с XML-библиотеками своих языков: JDK DOM (`javax.xml`), `XmlDocument` и `XDocument`, lxml и `xml.etree`, ext/dom и SimpleXML. Все программы читают одни и те же файлы из `wrappers/data` (каталог, схема, запрос), а память каждой библиотеки меряется в отдельном процессе (`--memory N`, Linux). Сначала соберите ядро с нужными обёртками (`make JAVA=1 CSHARP=1 PYTHON=1 PHP=1`), затем из каталога `benchmarks/wrappers`:

```bash
node make_data.js 50000
```
```bash
javac --release 22 -d /tmp/wj $(find ../../bindings/java/src/main/java java/src -name '*.java') && cp -r ../../bindings/java/src/main/resources/natives /tmp/wj/ && java --enable-native-access=ALL-UNNAMED -cp /tmp/wj bench.WrapperBench data
```
```bash
dotnet run -c Release --project csharp/WrapperBench.csproj -- data
```
```bash
PYTHONPATH=../../bindings/python python3 python/bench.py data
```

```bash
php -d ffi.enable=1 php/bench.php data
```

lxml необязательна (`pip install lxml`): без неё её строка пропускается. PHP-обёртка (`make PHP=1`) сравнивается с ext/dom (`DOMDocument` + `DOMXPath`, XSD — `schemaValidateSource`) и SimpleXML; память — `php php/bench.php data --memory N` (N — номер библиотеки, Linux).

Потоковая проверка по XSD без дерева сравнивается отдельными программами на тех же данных: `bench.StreamBench` (Java: «Берёста» против `javax.xml.validation.Validator`), `WrapperBench` с ключом `--stream` (.NET: против проверяющего `XmlReader`), `python/stream_bench.py` (Python: против lxml `iterparse` со схемой) и `php/stream_bench.php` (PHP: против `XMLReader` с `setSchema`). Запуск — как у основных программ, например `java ... bench.StreamBench data` или `dotnet run -c Release --project csharp/WrapperBench.csproj -- data --stream`.

Результаты и выводы — в разделе «Производительность» файла [docs/comparison.md](../docs/comparison.md#производительность).

---

# Performance measurements

The `beresta_bench` program compares Beresta with other XML libraries on the same data. The operations are the same for every library:
- parsing into DOM;
- walking the whole tree;
- serialization without indentation;
- an XPath 1.0 query;
- parsing with XML Schema validation (with building a tree);
- streaming XML Schema validation without a tree (SAX): Beresta — `SaxParser` with a schema, libxml2 — `xmlSchemaValidateStream`;
- the tree memory, measured in a separate process so that the libraries do not share a heap.

The compared libraries are included only if pkg-config found them (RapidXML — by its header). Beresta itself does not depend on them: this is only a comparison tool.

`wrappers/` compares the Java, .NET, Python and PHP wrappers with the XML libraries of their languages: the JDK DOM (`javax.xml`), `XmlDocument` and `XDocument`, lxml and `xml.etree`, ext/dom and SimpleXML. All programs read the same files from `wrappers/data` (`node make_data.js`), and the memory of every library is measured in a separate process (`--memory N`, Linux). The commands are given above. Streaming XSD validation without a tree is compared by separate programs on the same data: `bench.StreamBench` (Java: Beresta against `javax.xml.validation.Validator`), `WrapperBench` with the `--stream` switch (.NET: against a validating `XmlReader`), `python/stream_bench.py` (Python: against lxml `iterparse` with a schema) and `php/stream_bench.php` (PHP: against `XMLReader` with `setSchema`).

See the "Performance" section of [docs/comparison.en.md](../docs/comparison.en.md#performance) for the results.
