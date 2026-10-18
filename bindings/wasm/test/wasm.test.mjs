/*
 * Самопроверка пакета WebAssembly: тот же API, что у Node.js-обёртки, но
 * через beresta_core.wasm. Запуск: node --test (из bindings/wasm) или
 * make wasm-test (из корня репозитория).
 *
 * A self-test of the WebAssembly package: the same API as the Node.js wrapper, but
 * through beresta_core.wasm. Run: node --test (from bindings/wasm) or
 * make wasm-test (from the repository root).
 */
import test from 'node:test';
import assert from 'node:assert/strict';
import init from '../dist/beresta.mjs';

const beresta = await init();

// Тексты ошибок сверяются по-русски / Error texts are compared in Russian
beresta.setLanguage('ru');

const SCHEMA = "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
    + "<xs:element name='order'><xs:complexType><xs:sequence>"
    + "<xs:element name='item' maxOccurs='unbounded'><xs:complexType>"
    + "<xs:attribute name='id' type='xs:ID' use='required'/></xs:complexType></xs:element>"
    + '</xs:sequence></xs:complexType>'
    + "<xs:key name='k'><xs:selector xpath='item'/><xs:field xpath='@id'/></xs:key></xs:element></xs:schema>";

test('Wasm: разбор, навигация и вывод / Wasm: parsing, navigation and output', function parsing()
{
    const document = beresta.parse("<shop><item code='A1'>Чай</item><item code='B2'>Кофе</item></shop>");
    const item = document.root.firstChild;

    assert.equal(beresta.platformName(), 'wasm32');
    assert.match(beresta.version(), /^\d+\.\d+\.\d+$/);
    assert.equal(item.getAttribute('code'), 'A1');
    assert.equal(item.text, 'Чай');
    assert.equal(document.root.childCount, 2);
    item.setAttribute('code', 'A2');
    assert.equal(document.toXml(), "<shop><item code=\"A2\">Чай</item><item code=\"B2\">Кофе</item></shop>");
    document.close();
});

test('Wasm: байты в windows-1251 / Wasm: bytes in windows-1251', function encodings()
{
    const head = new TextEncoder().encode("<?xml version='1.0' encoding='windows-1251'?><a>");
    const bytes = new Uint8Array([...head, 0xD7, 0xE0, 0xE9, ...new TextEncoder().encode('</a>')]);
    const document = beresta.parse(bytes);

    assert.equal(document.root.text, 'Чай');
    document.close();
});

test('Wasm: XPath / Wasm: XPath', function xpath()
{
    const document = beresta.parse("<s:shop xmlns:s='urn:s'><s:item p='120'/><s:item p='80'/></s:shop>");
    const query = new beresta.XPath('//s:item[@p > $min]', { namespaces: { s: 'urn:s' }, variables: { min: 100 } });

    assert.equal(query.select(document).length, 1);
    assert.equal(new beresta.XPath('sum(//@p)').number(document), 200);
    assert.equal(new beresta.XPath('count(//*) = 3').boolean(document), true);
    query.close();
    document.close();
});

test('Wasm: XML Schema с ключами / Wasm: XML Schema with keys', function schema()
{
    const order = beresta.Schema.fromString(SCHEMA);
    const valid = beresta.parse("<order><item id='a'/><item id='b'/></order>");
    const invalid = beresta.parse("<order><item id='a'/><item id='a'/><item/></order>");

    assert.equal(order.validate(valid).length, 0);
    assert.ok(order.validate(invalid).length >= 2);
    assert.match(order.validate(invalid)[0].message, /^\/order\/item\[/);
    order.close();
    valid.close();
    invalid.close();
});

test('Wasm: перенос узла и ошибки / Wasm: moving a node and errors', function editing()
{
    const source = beresta.parse('<a><b>1</b></a>');
    const target = beresta.parse('<c/>');
    let parseError = null;
    let argumentError = null;

    source.root.firstChild.moveTo(target.root);
    source.close();
    assert.equal(target.toXml(), '<c><b>1</b></c>');
    try { beresta.parse('<a><b></a>'); } catch (error) { parseError = error; }
    try { beresta.parse(42); } catch (error) { argumentError = error; }
    assert.ok(parseError instanceof beresta.BerestaError);
    assert.equal(parseError.code, 'PARSE_ERROR');
    assert.equal(argumentError.name, 'TypeError');
    target.close();
});
