'use strict';

/*
 * Самопроверка Node.js-обёртки: XPath 1.0 и XML Schema.
 * A self-test of the Node.js binding: XPath 1.0 and XML Schema.
 */

const test = require('node:test');
const assert = require('node:assert/strict');
const beresta = require('../lib/index.js');

// Тексты ошибок сверяются по-русски — независимо от языка сборки
// Error texts are compared in Russian — regardless of the build language
beresta.setLanguage('ru');

const SHOP = "<s:shop xmlns:s='urn:shop'><s:item code='A1' price='120'>Чай</s:item>"
    + "<s:item code='B2' price='80'>Кофе</s:item></s:shop>";

const SCHEMA = "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
    + "<xs:element name='order'><xs:complexType><xs:sequence>"
    + "<xs:element name='qty' type='xs:positiveInteger'/></xs:sequence></xs:complexType></xs:element>"
    + '</xs:schema>';

test('XPath: значения / XPath: values', function xpathValues()
{
    const document = beresta.parse(SHOP);
    const query = new beresta.XPath('//s:item[@code = $c]', { namespaces: { s: 'urn:shop' }, variables: { c: 'B2' } });

    assert.equal(query.string(document), 'Кофе');
    query.set('c', 'A1');
    assert.equal(query.string(document), 'Чай');
    query.close();
    const sum = new beresta.XPath('sum(//@price)');
    assert.equal(sum.number(document), 200);
    assert.ok(Number.isNaN(new beresta.XPath("number('x')").number(document)));
    assert.equal(new beresta.XPath('//*[@price > $p]').set('p', 100).boolean(document), true);
    sum.close();
    document.close();
});

test('XPath: выборка / XPath: selection', function xpathSelection()
{
    const document = beresta.parse(SHOP);
    const items = new beresta.XPath('//p:item', { namespaces: { p: 'urn:shop' } }).select(document);
    let caught = null;

    assert.equal(items.length, 2);
    assert.ok(items[0] instanceof beresta.XmlElement);
    assert.equal(items[1].localName, 'item');
    assert.equal(items[1].namespaceUri, 'urn:shop');
    assert.deepEqual(new beresta.XPath('//@code').selectValues(document), ['A1', 'B2']);
    assert.deepEqual(new beresta.XPath('//@code').select(document), []);
    try { new beresta.XPath('//['); } catch (error) { caught = error; }
    assert.equal(caught.code, 'XPATH_ERROR');
    document.close();
});

test('XML Schema: список нарушений / XML Schema: violations', function schemaViolations()
{
    const schema = beresta.Schema.fromString(SCHEMA);
    const valid = beresta.parse('<order><qty>3</qty></order>');
    const invalid = beresta.parse('<order><qty>0</qty><extra/></order>');

    assert.deepEqual(schema.validate(valid), []);
    const problems = schema.validate(invalid);
    assert.ok(problems.length >= 2);
    assert.ok(problems[0] instanceof beresta.Diagnostic);
    assert.match(problems[0].message, /^\/order/);
    valid.close();
    invalid.close();
    schema.close();
    assert.equal(schema.closed, true);
});

test('XML Schema: проверка при разборе / XML Schema: validation while parsing', function schemaWhileParsing()
{
    const schema = beresta.Schema.fromString(Buffer.from(SCHEMA));
    let caught = null;

    try { beresta.parse('<order><qty>-1</qty></order>', { schema }); } catch (error) { caught = error; }
    assert.equal(caught.code, 'VALIDATION_ERROR');
    assert.match(caught.message, /строка 1/);
    const document = beresta.parse('<order><qty>7</qty></order>', { schema });
    assert.equal(document.root.firstChild.text, '7');
    document.close();
    schema.close();
});

/*
 * Освобождение через Symbol.dispose (то, что вызывает `using`), повторное
 * закрытие и сообщение о закрытом объекте.
 *
 * Releasing through Symbol.dispose (what `using` calls), a repeated close
 * and the message about a closed object.
 */
test('освобождение / disposal', function disposal()
{
    const document = beresta.parse('<a/>');
    const query = new beresta.XPath('/a');
    let caught = null;

    document[Symbol.dispose]();
    document.close();
    query[Symbol.dispose]();
    try { query.string(document); } catch (error) { caught = error; }
    assert.equal(document.closed, true);
    assert.ok(caught instanceof beresta.BerestaClosedError);
    assert.equal(caught.code, 'OBJECT_CLOSED');
});
