package org.beresta;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeStrings;
import org.beresta.internal.NodeFunctions;

/** Сериализация узла с потомками в строку. */
public interface NodeOutput extends NodeBase
{
    /** Компактный вывод без единого лишнего пробела. */
    default String toXml()
    {
        return toXml(WriteFlags.DEFAULT);
    }

    /** Вывод с флагами WriteFlags (например, PRETTY — с отступами). */
    default String toXml(int flags)
    {
        return NativeStrings.take(NativeCalls.invoke(NodeFunctions.TO_STRING, handle(), flags));
    }
}
