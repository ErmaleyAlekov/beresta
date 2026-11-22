'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const { native } = require('../lib/internal/native.js');
const beresta = require('../lib/index.js');

test('проверка аргументов аддона / addon argument validation', () => {
    const invalid = { code: 'INVALID_ARGUMENT', name: 'TypeError' };
    const doc = beresta.parse('<r/>');
    try {
        for (const value of [0.5, Infinity, -(2 ** 64), 2 ** 63])
            assert.throws(() => native.node_child(doc.handle, value), invalid);
        for (const value of [-1, 0.5, 2 ** 32, NaN])
            assert.throws(() => native.document_parse(Buffer.from('<r/>'), value, null), invalid);
        assert.throws(() => native.status_name(2 ** 32), invalid);
        assert.throws(() => native.init(null), invalid);
        assert.throws(() => beresta.parse('<'), beresta.BerestaError);
    } finally {
        doc.close();
    }
});
