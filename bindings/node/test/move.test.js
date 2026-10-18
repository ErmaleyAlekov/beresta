'use strict';

/*
 * Самопроверка переноса узлов между документами (moveTo).
 * A self-test of moving nodes between documents (moveTo).
 */

const test = require('node:test');
const assert = require('node:assert/strict');
const beresta = require('../lib/index.js');

// Тексты ошибок сверяются по-русски — независимо от языка сборки
// Error texts are compared in Russian — regardless of the build language
beresta.setLanguage('ru');

// Узел переносится из A в B, A закрывается, узлом и его потомком (обёртка получена ДО переноса) пользуются в B
// A node is moved from A to B, A is closed, the node and its descendant (the wrapper was obtained BEFORE the move) are used in B
test('перенос между документами / moving between documents', function moveBetweenDocuments()
{
    const source = beresta.parse("<a><item n='1'><name>Мёд</name></item><rest/></a>");
    const target = beresta.parse('<b/>');
    const item = source.root.firstChild;
    const name = item.firstChild;

    assert.equal(item, source.root.firstChild);
    item.moveTo(target.root);
    assert.equal(source.toXml(), '<a><rest/></a>');
    source.close();
    assert.equal(item.document, target);
    assert.equal(name.document, target);
    assert.equal(name.text, 'Мёд');
    item.setAttribute('n', '2');
    assert.equal(item.parent, target.root);
    assert.equal(target.toXml(), '<b><item n="2"><name>Мёд</name></item></b>');
    target.close();
});

// После закрытия B перенесённый узел недоступен, хотя A открыт / After B is closed the moved node is unavailable although A is open
test('перенос: закрытый новый документ / moving: a closed new document', function closedTargetDocument()
{
    const source = beresta.parse('<a><item><name/></item></a>');
    const target = beresta.parse('<b/>');
    const item = source.root.firstChild;
    const name = item.firstChild;
    let caught = null;

    item.moveTo(target.root);
    target.close();
    try { name.name; } catch (error) { caught = error; }
    assert.ok(caught instanceof beresta.BerestaClosedError);
    assert.equal(source.toXml(), '<a/>');
    source.close();
});

// Перенос, который обязан завершиться ошибкой с кодом code (try/catch — без вложенных функций)
// A move that must fail with the code code (try/catch — without nested functions)
function expectCode(node, parent, code)
{
    let caught = null;

    try { node.moveTo(parent); } catch (error) { caught = error; }
    assert.ok(caught instanceof beresta.BerestaError);
    assert.equal(caught.code, code);
}

// Запрещённые переносы: корень, документ, цикл; дерево не меняется / Forbidden moves: the root, a document, a cycle; the tree does not change
test('перенос: запрещённые случаи / moving: forbidden cases', function moveErrors()
{
    const document = beresta.parse('<r><p><q/></p></r>');
    const p = document.root.firstChild;

    expectCode(document.root, p, 'HIERARCHY_VIOLATION');
    expectCode(p, p.firstChild, 'HIERARCHY_VIOLATION');
    expectCode(document, p, 'INVALID_ARGUMENT');
    assert.equal(document.toXml(), '<r><p><q/></p></r>');
    assert.equal(p.document, document);
    document.close();
});
