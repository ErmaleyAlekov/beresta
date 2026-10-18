'use strict';

/*
 * Берёста — быстрая XML-библиотека DOM на C++17 для Node.js.
 * Вызовы идут через собственный аддон Node-API (без node-gyp и без
 * npm-зависимостей): бинарник для своей платформы лежит в native/<os>-<arch>/.
 *
 * Beresta is a fast XML DOM library in C++17 for Node.js.
 * The calls go through an own Node-API addon (without node-gyp and without
 * npm dependencies): the binary for each platform lives in native/<os>-<arch>/.
 */

const { native, platformName } = require('./internal/native.js');
const { BerestaError, BerestaClosedError, BerestaLoadError } = require('./errors.js');
const { ParseFlags, WriteFlags, NodeType } = require('./flags.js');
const { setLanguage, language, languages } = require('./languages.js');
const { XmlNode } = require('./nodes/node.js');
const { XmlElement } = require('./nodes/element.js');
const { XmlDocument } = require('./nodes/document.js');
const { parse, parseFile, createDocument } = require('./factory.js');
const { XPath } = require('./xpath/xpath.js');
const { Schema } = require('./schema/schema.js');
const { Diagnostic } = require('./schema/diagnostics.js');

// Версия нативной библиотеки, например "2.1.0" / The native library version, for example "2.1.0"
function version()
{
    return native.version();
}

module.exports = {
    version, platformName,
    parse, parseFile, createDocument,
    XmlNode, XmlElement, XmlDocument, XPath, Schema, Diagnostic,
    ParseFlags, WriteFlags, NodeType,
    setLanguage, language, languages,
    BerestaError, BerestaClosedError, BerestaLoadError
};
