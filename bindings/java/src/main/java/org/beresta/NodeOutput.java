package org.beresta;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeStrings;
import org.beresta.internal.NodeFunctions;

/** Сериализация узла с потомками в строку. / Serializes a node with its descendants into a string. */
public interface NodeOutput extends NodeBase
{
    /** Компактный вывод без единого лишнего пробела. / Compact output without a single extra space. */
    default String toXml()
    {
        return toXml(WriteFlags.DEFAULT);
    }

    /**
     * Вывод с флагами WriteFlags (например, PRETTY — с отступами).
     * <p>Output with WriteFlags (for example, PRETTY — with indentation).
     */
    default String toXml(int flags)
    {
        return NativeStrings.take(NativeCalls.invoke(NodeFunctions.TO_STRING, handle(), flags));
    }
}
