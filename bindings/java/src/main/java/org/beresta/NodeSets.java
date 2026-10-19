package org.beresta;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.util.ArrayList;
import java.util.List;

import org.beresta.internal.NodeCalls;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeStrings;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.XPathFunctions;

/**
 * Чтение результата beresta_xpath_select. Набор узлов живёт только внутри
 * метода: элементы копируются в список Java, нативный набор освобождается
 * в finally — даже если чтение прервалось исключением.
 *
 * <p>Reads the result of beresta_xpath_select. The node set lives only inside the
 * method: items are copied into a Java list, the native set is freed in finally —
 * even if reading was interrupted by an exception.
 */
final class NodeSets
{
    private NodeSets()
    {
    }

    /**
     * Выполнение select: нативный набор узлов (освобождает вызывающий).
     * <p>Runs select: a native node set (the caller frees it).
     */
    private static MemorySegment select(MemorySegment xpath, XmlNode context)
    {
        try (Arena arena = Arena.ofConfined())
        {
            MemorySegment out = NativeMemory.outPointer(arena);

            NativeCalls.check(NativeCalls.invoke(XPathFunctions.SELECT, xpath, context.handle(), out));
            return NativeMemory.readPointer(out);
        }
    }

    /**
     * Узлы набора (атрибуты пропускаются: у них нет узла DOM).
     * <p>Nodes of the set (attributes are skipped: they have no DOM node).
     */
    static List<XmlNode> nodes(MemorySegment xpath, XmlNode context)
    {
        MemorySegment set = select(xpath, context);
        List<XmlNode> result = new ArrayList<>();

        try
        {
            long size = (long) NativeCalls.invoke(XPathFunctions.NODESET_SIZE, set);
            for (long index = 0; index < size; ++index)
            {
                XmlNode node = NodeFactory.wrap(NodeCalls.indexed(XPathFunctions.NODESET_NODE, set, index), context.document());
                if (node != null)
                    result.add(node);
            }
            return result;
        }
        finally
        {
            NativeCalls.invoke(XPathFunctions.NODESET_FREE, set);
        }
    }

    /** Строковые значения всех элементов набора. / String values of all items of the set. */
    static List<String> values(MemorySegment xpath, XmlNode context)
    {
        MemorySegment set = select(xpath, context);
        List<String> result = new ArrayList<>();

        try
        {
            long size = (long) NativeCalls.invoke(XPathFunctions.NODESET_SIZE, set);
            for (long index = 0; index < size; ++index)
                result.add(NativeStrings.take(NativeCalls.invoke(XPathFunctions.NODESET_VALUE, set, index)));
            return result;
        }
        finally
        {
            NativeCalls.invoke(XPathFunctions.NODESET_FREE, set);
        }
    }
}
