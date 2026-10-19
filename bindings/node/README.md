# Берёста для Node.js

Быстрая XML-библиотека на C++17: DOM, XPath 1.0, XML Schema с `xs:key`/`xs:keyref`, DTD, восемь кодировок. Вызовы идут через собственный аддон Node-API: **нет ни npm-зависимостей, ни node-gyp, ни компиляции на машине пользователя**. Готовые типы TypeScript входят в пакет.

```bash
npm install beresta
```

```js
const beresta = require('beresta');

using document = beresta.parse("<shop><item code='A1'>Чай</item></shop>");
const item = document.root.firstChild;
console.log(item.getAttribute('code'), item.text);                        // A1 Чай

using query = new beresta.XPath('//item[@code = $c]', { variables: { c: 'A1' } });
console.log(query.string(document));                                      // Чай
```

`using` работает в Node.js 24+; в более ранних версиях вызывайте `document.close()` (если забыть, память освободит сборщик мусора).

Пакет из npm уже содержит аддоны для Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) и macOS (универсальный: Intel и Apple Silicon). Для браузера и Cloudflare Workers есть пакет [`beresta-wasm`](../wasm/README.md) с тем же API. Сборка из исходников: `make NODE=1` или `cmake -DBERESTA_BUILD_NODE=ON` в корне репозитория — аддон окажется в `native/<os>-<arch>/beresta.node`.

**Как получить скорость C++.** Каждый шаг ручного обхода (`firstChild`, `nextSibling`) — вызов через Node-API и объект-обёртка. Тяжёлую работу отдавайте ядру целиком:

```js
using schema = beresta.Schema.fromFile('export.xsd');
const problems = schema.validateFile('export.xml');                         // поток без дерева, хоть гигабайты
const prices = new beresta.XPath('//item/@price').selectValues(document);   // строки без объектов-узлов
```

Среди библиотек для Node.js с настоящим DOM, XPath и XSD «Берёста» быстрее всех, а потоковая проверка `validateFile` / `validateBytes` не строит дерево и отображает файл в память. Строка и столбец нарушений в `schema.validate(document)` — с флагом `ParseFlags.TRACK_LOCATIONS` при разборе.

---

# Beresta for Node.js

A fast C++17 XML library: DOM, XPath 1.0, XML Schema with `xs:key`/`xs:keyref`, DTD, eight encodings. The calls go through an own Node-API addon: **no npm dependencies, no node-gyp, no compilation on the user's machine**. TypeScript types are included.

```bash
npm install beresta
```

`using` works in Node.js 24+; in earlier versions call `document.close()` (if you forget, the garbage collector frees the memory).

The npm package already contains the addons for Windows (x64, arm64), Linux (x64, arm64; glibc 2.35+) and macOS (universal: Intel and Apple Silicon). For a browser and Cloudflare Workers there is the [`beresta-wasm`](../wasm/README.md) package with the same API. Building from source: `make NODE=1` or `cmake -DBERESTA_BUILD_NODE=ON` in the repository root — the addon lands in `native/<os>-<arch>/beresta.node`.

**How to get C++ speed.** Every step of a manual walk (`firstChild`, `nextSibling`) is a call through Node-API and a wrapper object. Hand heavy work over to the core entirely:

```js
using schema = beresta.Schema.fromFile('export.xsd');
const problems = schema.validateFile('export.xml');                         // a stream without a tree, even gigabytes
const prices = new beresta.XPath('//item/@price').selectValues(document);   // strings without node objects
```

Among the Node.js libraries with a real DOM, XPath and XSD Beresta is the fastest, and streaming validation with `validateFile` / `validateBytes` builds no tree and maps the file into memory. The line and column of violations in `schema.validate(document)` come with the `ParseFlags.TRACK_LOCATIONS` parse flag.
