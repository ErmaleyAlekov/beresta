'use strict';

/*
 * Фабрика объектов-обёрток для дескрипторов узлов (паттерн Factory Method).
 * Си-интерфейс отдаёт «голый» дескриптор; по типу узла выбирается класс:
 * XmlElement для элементов, сам документ для дескриптора документа,
 * XmlNode для остальных узлов. Классы регистрируются из element.js:
 * так нет циклического require между узлом, его примесями и фабрикой.
 * Уже выданная живая обёртка того же узла возвращается повторно (реестр
 * документа, паттерн Identity Map, см. registry.js): у узла одна обёртка.
 *
 * The factory of wrapper objects for node handles (the Factory Method pattern).
 * The C interface returns a "bare" handle; the class is chosen by the node type:
 * XmlElement for elements, the document itself for the document handle,
 * XmlNode for other nodes. The classes are registered from element.js:
 * so there is no circular require between a node, its mixins and the factory.
 * A live wrapper already handed out for the same node is returned again (the
 * document registry, the Identity Map pattern, see registry.js): one node, one wrapper.
 */

const { native } = require('../internal/native.js');
const { HANDLE, REGISTRY } = require('../internal/keys.js');
const { NodeType } = require('../flags.js');

// Зарегистрированные классы узлов / The registered node classes
const classes = { node: null, element: null };

// Регистрация классов (вызывается один раз из element.js) / Registers the classes (called once from element.js)
function registerNodeClasses(nodeClass, elementClass)
{
    classes.node = nodeClass;
    classes.element = elementClass;
}

// Обёртка для дескриптора узла документа document (null -> null)
// A wrapper for a node handle of the document document (null -> null)
function wrap(document, handle)
{
    if (handle === null)
        return null;
    if (handle === document[HANDLE])
        return document;
    const element = native.node_type(handle) === NodeType.ELEMENT;
    let node = document[REGISTRY].find(handle, element);
    if (node !== null)
        return node;
    if (element)
        node = new classes.element(document, handle);
    else
        node = new classes.node(document, handle);
    document[REGISTRY].store(handle, node, element);
    return node;
}

module.exports = { registerNodeClasses, wrap };
