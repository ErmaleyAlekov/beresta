# Берёста для WebAssembly

XML-библиотека «Берёста» в WebAssembly: DOM, полный XPath 1.0, XML Schema с `xs:key`/`xs:keyref`, DTD и восемь кодировок — **в браузере, Web Worker, Cloudflare Workers и Node.js**. Зависимостей нет; модуль весит около 650 КБ (около 250 КБ в gzip).

```bash
npm install beresta-wasm
```

```js
import init from 'beresta-wasm';

const beresta = await init();
const order = beresta.parse("<order><item id='a' price='120'/><item id='b' price='80'/></order>");
console.log(new beresta.XPath('sum(//@price)').number(order));      // 200

const schema = beresta.Schema.fromString(xsdText);
for (const problem of schema.validate(order))
    console.log(problem.message);                                     // /order/item[2]: ...
order.close();
```

API тот же, что у пакета [`beresta`](../node/README.md) для Node.js: классы те же, только доступны после `await init()`. Типы TypeScript входят в пакет.

**Где лежит `.wasm`.** Сборщики (Vite, webpack, esbuild) находят `beresta_core.wasm` сами — рядом с `beresta.mjs`. Если файл лежит в другом месте, передайте `init({ locateFile: function locate(name) { return '/static/' + name; } })` или уже загруженные байты: `init({ wasmBinary })`.

**Cloudflare Workers.** Модуль Wasm подключается импортом и передаётся в `init` через `instantiateWasm` (так подключают модули Emscripten в Workers; в CI этого репозитория проверяются браузер и Node.js, а Workers — нет):

```js
import init from 'beresta-wasm';
import wasmModule from 'beresta-wasm/beresta_core.wasm';

function instantiateWasm(imports, done)
{
    WebAssembly.instantiate(wasmModule, imports).then(function ready(instance) { done(instance, wasmModule); });
    return {};
}

export default {
    async fetch(request)
    {
        const beresta = await init({ instantiateWasm });
        const document = beresta.parse(await request.text());
        return new Response(document.root.name);
    }
};
```

**Ограничения.** Файловой системы в WebAssembly нет: `parseFile`, `save` и `Schema.fromFile` недоступны — разбирайте строки и байты, выводите в строку (`toXml`). Скорость разбора — около 65 % от нативного аддона Node.js, XPath работает так же быстро.

Пример страницы — [`examples/browser.html`](examples/browser.html). Сборка из исходников: `make wasm` (нужен Emscripten), самопроверка — `make wasm-test`.

**Проверка без дерева.** `schema.validateBytes(bytes)` проверяет документ потоком, не строя DOM: так удобно проверять большой файл, выбранный в `<input type="file">`, прямо в браузере. Файловой системы в WebAssembly нет, поэтому `validateFile` даёт `IO_ERROR`; используйте байты.

---

# Beresta for WebAssembly

The Beresta XML library in WebAssembly: DOM, full XPath 1.0, XML Schema with `xs:key`/`xs:keyref`, DTD and eight encodings — **in a browser, a Web Worker, Cloudflare Workers and Node.js**. No dependencies; the module weighs about 650 KB (about 250 KB gzipped).

```bash
npm install beresta-wasm
```

The API is the same as in the [`beresta`](../node/README.md) package for Node.js: the classes are the same, they are just available after `await init()`. TypeScript types are included.

**Where the `.wasm` lies.** Bundlers (Vite, webpack, esbuild) find `beresta_core.wasm` themselves — next to `beresta.mjs`. If the file lies elsewhere, pass `init({ locateFile: function locate(name) { return '/static/' + name; } })` or already loaded bytes: `init({ wasmBinary })`. In **Cloudflare Workers** import the module (`import wasmModule from 'beresta-wasm/beresta_core.wasm'`) and pass it through `instantiateWasm` (see the example above; the CI of this repository checks a browser and Node.js, but not Workers).

**Limitations.** WebAssembly has no file system: `parseFile`, `save` and `Schema.fromFile` are unavailable — parse strings and bytes, output to a string (`toXml`). Parsing speed is about 65 % of the native Node.js addon, XPath is just as fast.

An example page is [`examples/browser.html`](examples/browser.html). Building from source: `make wasm` (Emscripten is required), the self-test is `make wasm-test`.

**Validation without a tree.** `schema.validateBytes(bytes)` validates a document as a stream without building a DOM: this is convenient for checking a large file chosen in `<input type="file">` right in the browser. WebAssembly has no file system, so `validateFile` gives `IO_ERROR`; use bytes.
