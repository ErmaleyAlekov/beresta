package org.beresta;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;
import java.util.List;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeStrings;
import org.beresta.internal.XPathFunctions;

/**
 * Вычисление выражения относительно узла (или документа).
 * <p>Evaluates the expression relative to a node (or a document).
 */
public interface XPathEvaluation extends XPathBase
{
    /**
     * Узлы результата в порядке документа. Атрибуты в DOM «Берёсты» —
     * не узлы и сюда не попадают; их значения возвращает selectValues.
     *
     * <p>Result nodes in document order. Attributes in the Beresta DOM are
     * not nodes and are not included here; selectValues returns their values.
     */
    default List<XmlNode> selectNodes(XmlNode context)
    {
        return NodeSets.nodes(handle(), context);
    }

    /**
     * Строковые значения всех элементов результата, включая атрибуты.
     * <p>String values of all result items, attributes included.
     */
    default List<String> selectValues(XmlNode context)
    {
        return NodeSets.values(handle(), context);
    }

    /** Результат по правилам функции string(). / The result by the rules of the string() function. */
    default String string(XmlNode context)
    {
        return NativeStrings.take(NativeCalls.invoke(XPathFunctions.STRING, handle(), context.handle()));
    }

    /**
     * Результат по правилам функции number(); NaN — не ошибка.
     * <p>The result by the rules of the number() function; NaN is not an error.
     */
    default double number(XmlNode context)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = arena.allocate(ValueLayout.JAVA_DOUBLE);

            NativeCalls.check(NativeCalls.invoke(XPathFunctions.NUMBER, handle(), context.handle(), out));
            return out.get(ValueLayout.JAVA_DOUBLE, 0);
        }
    }

    /** Результат по правилам функции boolean(). / The result by the rules of the boolean() function. */
    default boolean bool(XmlNode context)
    {
        int result = (int) NativeCalls.invoke(XPathFunctions.BOOLEAN, handle(), context.handle());

        if (result < 0)
            throw NativeCalls.lastError();
        return result == 1;
    }
}
