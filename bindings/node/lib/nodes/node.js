'use strict';

const { applyMixins } = require('../internal/mixin.js');
const { NodeBase } = require('./node_base.js');
const { NodeContent } = require('./node_content.js');
const { NodeNavigation } = require('./node_navigation.js');
const { NodeChildren } = require('./node_children.js');
const { NodeEditing } = require('./node_editing.js');
const { NodeStructure } = require('./node_structure.js');

/**
 * Узел дерева документа. Класс лишь собирает части: основу (дескриптор и
 * документ) и примеси содержимого, навигации, детей, изменения и структуры.
 *
 * A node of the document tree. The class only combines the parts: the base
 * (the handle and the document) and the mixins of content, navigation,
 * children, editing and structure.
 */
class XmlNode extends NodeBase
{
}

applyMixins(XmlNode, NodeContent, NodeNavigation, NodeChildren, NodeEditing, NodeStructure);

module.exports = { XmlNode };
