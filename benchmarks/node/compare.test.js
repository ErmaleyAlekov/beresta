'use strict';

const assert = require('node:assert/strict');
const test = require('node:test');
const { measureEngine } = require('./compare.js');

// Тестовый движок отслеживает владение документами. / The test engine tracks document ownership.
function trackingEngine(failOnWalk)
{
    const live = new Set();
    return {
        name: 'Test engine',
        live,
        parse()
        {
            assert.equal(live.size, 0);
            const document = {};
            live.add(document);
            return document;
        },
        walk(document)
        {
            assert.ok(live.has(document));
            if (failOnWalk)
                throw new Error('walk failed');
            return 1;
        },
        release(document)
        {
            assert.ok(live.delete(document));
        }
    };
}

test('releases each parse document before the next parse', function ()
{
    const engine = trackingEngine(false);
    measureEngine(engine, '<r/>');
    assert.equal(engine.live.size, 0);
});

test('releases the final document when walking fails', function ()
{
    const engine = trackingEngine(true);
    assert.throws(function () { measureEngine(engine, '<r/>'); }, /walk failed/);
    assert.equal(engine.live.size, 0);
});
