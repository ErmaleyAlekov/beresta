# Берёста для Node.js

Быстрая XML-библиотека на C++17: DOM, XPath 1.0, XML Schema, DTD, восемь кодировок. Вызовы идут через собственный аддон Node-API: **нет ни npm-зависимостей, ни node-gyp, ни компиляции на машине пользователя**. Готовые типы TypeScript входят в пакет.

```js
const beresta = require('beresta');

using document = beresta.parse("<shop><item code='A1'>Чай</item></shop>");
const item = document.root.firstChild;
console.log(item.getAttribute('code'), item.text);                        // A1 Чай

using query = new beresta.XPath('//item[@code = $c]', { variables: { c: 'A1' } });
console.log(query.string(document));                                      // Чай
```

`using` работает в Node.js 24+; в более ранних версиях вызывайте `document.close()` (если забыть, память освободит сборщик мусора).

Перед использованием соберите аддон из корня репозитория: `make NODE=1` или `cmake -DBERESTA_BUILD_NODE=ON`. Бинарник окажется в `native/<os>-<arch>/beresta.node`. Подробности — в README.md в корне репозитория.

---

# Beresta for Node.js

A fast C++17 XML library: DOM, XPath 1.0, XML Schema, DTD, eight encodings. The calls go through an own Node-API addon: **no npm dependencies, no node-gyp, no compilation on the user's machine**. TypeScript types are included.

Build the addon from the repository root first: `make NODE=1` or `cmake -DBERESTA_BUILD_NODE=ON`. The binary lands in `native/<os>-<arch>/beresta.node`.
