'use strict';

/*
 * Самопроверка Node.js-обёртки: разбор, навигация, изменение дерева, ошибки.
 * Только встроенные модули node:test и node:assert — фреймворки не нужны.
 *
 * A self-test of the Node.js binding: parsing, navigation, tree changes, errors.
 * Only the built-in node:test and node:assert modules — no frameworks are needed.
 */

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const beresta = require('../lib/index.js');

// Тексты ошибок сверяются по-русски — независимо от языка сборки
// Error texts are compared in Russian — regardless of the build language
beresta.setLanguage('ru');

test('разбор и навигация / parsing and navigation', function parsingAndNavigation()
{
    const document = beresta.parse("<shop><item code='A1'>Чай</item><!--x--><item>Кофе</item></shop>");
    const root = document.root;
    const names = [];

    for (const child of root)
        names.push(child.name);
    assert.deepEqual(names, ['item', '#comment', 'item']);
    assert.equal(root.name, 'shop');
    assert.equal(root.childCount, 3);
    assert.equal(root.child(0).getAttribute('code'), 'A1');
    assert.equal(root.child(-1).text, 'Кофе');
    assert.equal(root.child(1).type, beresta.NodeType.COMMENT);
    assert.ok(root.firstChild.nextSibling.nextSibling.parent.isSameNode(root));
    assert.equal(root.parent, document);
    assert.equal(root.children.length, 3);
    document.close();
});

test('построение и вывод / building and output', function buildingAndOutput()
{
    const document = beresta.createDocument('order');
    const item = document.root.appendElement('item').setAttribute('qty', 2);

    item.appendText('Мёд & орехи');
    assert.equal(document.toXml(), '<order><item qty="2">Мёд &amp; орехи</item></order>');
    assert.match(document.toXml(beresta.WriteFlags.PRETTY), /\n {4}<item/);
    assert.deepEqual(item.attributes, { qty: '2' });
    assert.equal(item.removeAttribute('qty'), true);
    assert.equal(item.hasAttribute('qty'), false);
    item.text = 'новый';
    assert.equal(item.toXml(), '<item>новый</item>');
    document.close();
});

test('копирование и удаление / copying and removal', function copyingAndRemoval()
{
    const source = beresta.parse("<a><b x='1'/></a>");
    const target = beresta.parse('<c/>');

    source.root.firstChild.copyTo(target.root);
    source.root.firstChild.remove();
    const copy = target.clone();
    target.close();
    assert.equal(source.toXml(), '<a/>');
    assert.equal(copy.toXml(), '<c><b x="1"/></c>');
    assert.equal(target.closed, true);
    source.close();
    copy.close();
});

/*
 * Ошибки ловятся обычным try/catch: так проверка не требует вложенных
 * функций-обёрток вокруг каждого вызова.
 *
 * Errors are caught with an ordinary try/catch: this way a check needs no
 * nested wrapper functions around each call.
 */
test('ошибки / errors', function errors()
{
    const caught = [];
    const document = beresta.parse('<a/>');
    const root = document.root;

    try { beresta.parse('<a><b></a>'); } catch (error) { caught.push(error); }
    try { root.appendElement('1bad'); } catch (error) { caught.push(error); }
    try { beresta.parse(42); } catch (error) { caught.push(error); }
    document.close();
    try { root.text; } catch (error) { caught.push(error); }
    assert.equal(caught.length, 4);
    assert.ok(caught[0] instanceof beresta.BerestaError);
    assert.equal(caught[0].code, 'PARSE_ERROR');
    assert.equal(caught[0].status, 6);
    assert.match(caught[0].message, /^строка 1, столбец 7/);
    assert.equal(caught[1].code, 'INVALID_NAME');
    assert.ok(caught[2] instanceof TypeError);
    assert.ok(caught[3] instanceof beresta.BerestaClosedError);
});

test('файлы и кодировки / files and encodings', function filesAndEncodings()
{
    const folder = fs.mkdtempSync(path.join(os.tmpdir(), 'beresta-'));
    const file = path.join(folder, 'документ.xml');
    const document = beresta.parse('<текст>Привет</текст>');

    document.save(file, { encoding: 'windows-1251' });
    document.close();
    assert.ok(fs.readFileSync(file).includes('windows-1251'));
    const loaded = beresta.parseFile(file, { flags: beresta.ParseFlags.DISCARD_COMMENTS });
    assert.equal(loaded.root.text, 'Привет');
    loaded.close();
    const bytes = beresta.parse(fs.readFileSync(file));
    assert.equal(bytes.root.name, 'текст');
    bytes.close();
    fs.rmSync(folder, { recursive: true, force: true });
});
