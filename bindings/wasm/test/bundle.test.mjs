import assert from 'node:assert/strict';
import test from 'node:test';
import { dependencies } from '../tools/dependencies.mjs';

test('зависимости вне комментариев и строк / dependencies outside comments and strings', () => {
    const source = [
        "// require('./line')",
        "/* require('./block') */",
        "\"require('./string')\"",
        "`require('./template')`",
        "require('./a')",
        'require("./b")'
    ].join('\n');
    const calls = [...dependencies(source)];
    assert.deepEqual(calls.map(match => match[2]), ['./a', './b']);
    for (const match of calls)
        assert.equal(source.slice(match.index, match.index + match[0].length), match[0]);
});
