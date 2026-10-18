package org.beresta;

import java.util.ArrayList;
import java.util.List;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NodeFunctions;

/** Доступ к детям по индексу и списком. / Access to children by index and as a list. */
public interface NodeChildren extends NodeBase
{
    /** Число детей (0 у листовых узлов). / The number of children (0 for leaf nodes). */
    default long childCount()
    {
        long count = (long) NativeCalls.invoke(NodeFunctions.CHILD_COUNT, handle());

        if (count < 0)
            throw NativeCalls.lastError();
        return count;
    }

    /**
     * Ребёнок по индексу; неверный индекс — BerestaException(INVALID_ARGUMENT).
     * <p>A child by index; an invalid index throws BerestaException(INVALID_ARGUMENT).
     */
    default XmlNode child(long index)
    {
        return NodeFactory.wrap(NativeCalls.invoke(NodeFunctions.CHILD, handle(), index), document());
    }

    /**
     * Снимок списка детей: обход через соседей, O(1) на шаг.
     * <p>A snapshot of the child list: traversal through siblings, O(1) per step.
     */
    default List<XmlNode> children()
    {
        List<XmlNode> result = new ArrayList<>();
        XmlNode child = NodeFactory.wrap(NativeCalls.invoke(NodeFunctions.FIRST_CHILD, handle()), document());

        while (child != null)
        {
            result.add(child);
            child = child.nextSibling();
        }
        return result;
    }
}
