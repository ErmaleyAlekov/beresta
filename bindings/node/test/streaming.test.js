'use strict';

/*
 * Самопроверка потоковой проверки по схеме и массового переноса узлов.
 * A self-test of streaming schema validation and bulk node moves.
 */

const test = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const beresta = require('../lib/index.js');

beresta.setLanguage('ru');

const ORDER_XSD = "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
    + "<xs:element name='order'><xs:complexType><xs:sequence>"
    + "<xs:element name='item' maxOccurs='unbounded'><xs:complexType><xs:simpleContent>"
    + "<xs:extension base='xs:string'><xs:attribute name='qty' type='xs:positiveInteger' use='required'/>"
    + '</xs:extension></xs:simpleContent></xs:complexType></xs:element>'
    + '</xs:sequence></xs:complexType></xs:element></xs:schema>';

const ORDER_XML = "<order>\n<item qty='2'>чай</item>\n<item qty='0'>кофе</item>\n<item>мёд</item>\n</order>";

// Код ошибки проверки файла file или (null) битых байтов; try/catch — без вложенных функций
// The error code of validating the file file or (null) broken bytes; try/catch — without nested functions
function codeOf(schema, file)
{
    let caught = null;

    try
    {
        if (file === null)
            schema.validateBytes("<order><item qty='1'></order>");
        else
            schema.validateFile(file);
    }
    catch (error)
    {
        caught = error;
    }
    assert.ok(caught instanceof beresta.BerestaError);
    return caught.code;
}

// Поток = дерево с позициями; без TRACK_LOCATIONS места у дерева — 0 / Stream = a tree with positions; without TRACK_LOCATIONS the tree's places are 0
test('XSD поток: совпадает с деревом / XSD stream: matches the tree', function streamMatchesTree()
{
    const schema = beresta.Schema.fromString(ORDER_XSD);
    const located = beresta.parse(ORDER_XML, { flags: beresta.ParseFlags.TRACK_LOCATIONS });
    const plain = beresta.parse(ORDER_XML);
    const stream = schema.validateBytes(ORDER_XML);

    assert.deepEqual(schema.validate(located), stream);
    assert.equal(stream.length, 2);
    assert.equal(stream[0].line, 3);
    assert.ok(stream[0].message.startsWith('/order/item[2]'));
    assert.equal(schema.validate(plain)[0].line, 0);
    assert.deepEqual(schema.validateBytes(Buffer.from("<order><item qty='1'/></order>")), []);
    located.close();
    plain.close();
    schema.close();
});

// Файл с кириллическим именем, отсутствующий файл и синтаксическая ошибка / A Cyrillic file name, a missing file and a syntax error
test('XSD поток: файлы и ошибки / XSD stream: files and errors', function streamFile()
{
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'beresta-stream-'));
    const file = path.join(directory, 'выгрузка.xml');
    const schema = beresta.Schema.fromString(ORDER_XSD);

    fs.writeFileSync(file, ORDER_XML, 'utf8');
    assert.deepEqual(schema.validateFile(file), schema.validateBytes(ORDER_XML));
    assert.equal(codeOf(schema, path.join(directory, 'нет.xml')), 'IO_ERROR');
    assert.equal(codeOf(schema, null), 'PARSE_ERROR');
    schema.close();
    fs.rmSync(directory, { recursive: true });
});

// 50 000 переносов с конца: обёртки те же (реестр), A закрыт / 50,000 moves from the end: the same wrappers (registry), A is closed
test('50 000 переносов и реестр обёрток / 50,000 moves and the wrapper registry', function bulkMove()
{
    const parts = ['<a>'];
    for (let index = 0; index < 50000; ++index)
        parts.push('<n>' + index + '</n>');
    const source = beresta.parse(parts.join('') + '</a>');
    const target = beresta.parse('<b/>');
    const wrappers = source.root.children;

    for (let index = wrappers.length - 1; index >= 0; --index)
        wrappers[index].moveTo(target.root);
    source.close();
    assert.equal(target.root.firstChild, wrappers[49999]);
    assert.equal(wrappers[123].document, target);
    assert.equal(wrappers[123].text, '123');
    assert.equal(new beresta.XPath('count(/b/n)').number(target), 50000);
    target.close();
});
