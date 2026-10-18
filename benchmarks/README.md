# Замеры производительности

Программа `beresta_bench` сравнивает «Берёсту» с другими XML-библиотеками на одинаковых данных. Операции одинаковы для всех библиотек:
- разбор в DOM;
- обход всего дерева;
- сериализация без отступов;
- запрос XPath 1.0;
- разбор с проверкой по XML Schema;
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

Результаты и выводы — в разделе «Производительность» README.md в корне репозитория.

---

# Performance measurements

The `beresta_bench` program compares Beresta with other XML libraries on the same data. The operations are the same for every library:
- parsing into DOM;
- walking the whole tree;
- serialization without indentation;
- an XPath 1.0 query;
- parsing with XML Schema validation;
- the tree memory, measured in a separate process so that the libraries do not share a heap.

The compared libraries are included only if pkg-config found them (RapidXML — by its header). Beresta itself does not depend on them: this is only a comparison tool. See the "Производительность" section of the root README.md for the results.
