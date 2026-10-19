/*
 * Самопроверка потоковой проверки по схеме в пакете WebAssembly.
 * A self-test of streaming schema validation in the WebAssembly package.
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

// Потоковая проверка без дерева: те же нарушения, что у дерева; файлов в Wasm нет — IO_ERROR
// Streaming validation without a tree: the same violations as the tree has; Wasm has no files — IO_ERROR
test('Wasm: потоковая проверка / Wasm: streaming validation', function streaming()
{
    const order = beresta.Schema.fromString(SCHEMA);
    const xml = "<order><item id='a'/><item id='a'/><item/></order>";
    const invalid = beresta.parse(xml, { flags: beresta.ParseFlags.TRACK_LOCATIONS });
    let fileError = null;

    assert.deepEqual(order.validateBytes(xml), order.validate(invalid));
    assert.deepEqual(order.validateBytes(new TextEncoder().encode("<order><item id='a'/></order>")), []);
    try { order.validateFile('/нет/такого.xml'); } catch (error) { fileError = error; }
    assert.equal(fileError.code, 'IO_ERROR');
    assert.match(fileError.message, /файловой системы/);
    order.close();
    invalid.close();
});
