package org.beresta;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NodeFunctions;

/**
 * Навигация по дереву за O(1): ядро хранит индекс узла у родителя.
 * Отсутствие соседа или ребёнка — null.
 *
 * <p>O(1) tree navigation: the core stores the node's index in its parent.
 * A missing sibling or child is null.
 */
public interface NodeNavigation extends NodeBase
{
    default XmlNode parent()
    {
        return NodeFactory.wrap(NativeCalls.invoke(NodeFunctions.PARENT, handle()), document());
    }

    default XmlNode firstChild()
    {
        return NodeFactory.wrap(NativeCalls.invoke(NodeFunctions.FIRST_CHILD, handle()), document());
    }

    default XmlNode lastChild()
    {
        return NodeFactory.wrap(NativeCalls.invoke(NodeFunctions.LAST_CHILD, handle()), document());
    }

    default XmlNode nextSibling()
    {
        return NodeFactory.wrap(NativeCalls.invoke(NodeFunctions.NEXT_SIBLING, handle()), document());
    }

    default XmlNode previousSibling()
    {
        return NodeFactory.wrap(NativeCalls.invoke(NodeFunctions.PREVIOUS_SIBLING, handle()), document());
    }
}
