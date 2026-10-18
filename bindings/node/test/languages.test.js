'use strict';

/*
 * Самопроверка языков сообщений. / A self-test of message languages.
 */

const test = require('node:test');
const assert = require('node:assert/strict');
const beresta = require('../lib/index.js');

test('список языков / the language list', function languageList()
{
    assert.deepEqual(beresta.languages().slice(0, 2), ['ru', 'en']);
    assert.match(beresta.version(), /^\d+\.\d+\.\d+$/);
    assert.match(beresta.platformName(), /^[a-z]+-[a-z0-9_]+$/);
});

/*
 * Смена языка во время работы: и ошибки ядра, и сообщения самой обёртки
 * сразу приходят на английском; код ошибки от языка не зависит.
 *
 * Switching the language at run time: both core errors and the messages of
 * the binding itself come in English at once; the error code does not depend on the language.
 */
test('смена языка / switching the language', function switchLanguage()
{
    const document = beresta.parse('<a/>');
    const caught = [];

    beresta.setLanguage('en');
    try { beresta.parse('<a><b></a>'); } catch (error) { caught.push(error); }
    document.close();
    try { document.root; } catch (error) { caught.push(error); }
    beresta.setLanguage('ru');
    assert.equal(caught[0].message, 'line 1, column 7: expected </b>, found </a>');
    assert.equal(caught[0].code, 'PARSE_ERROR');
    assert.match(caught[1].message, /the document is already closed/);
    assert.equal(beresta.language(), 'ru');
});

test('неизвестный язык / an unknown language', function unknownLanguage()
{
    let caught = null;

    try { beresta.setLanguage('xx'); } catch (error) { caught = error; }
    assert.equal(caught.code, 'INVALID_ARGUMENT');
    assert.equal(beresta.language(), 'ru');
});
