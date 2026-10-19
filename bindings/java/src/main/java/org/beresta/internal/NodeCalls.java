package org.beresta.internal;

import java.lang.foreign.MemorySegment;
import java.lang.invoke.MethodHandle;

/**
 * Быстрые вызовы для горячих путей обхода дерева: навигация, тип узла,
 * ребёнок по индексу и узел результата XPath. Общий NativeCalls.invoke
 * идёт через invokeWithArguments: аргументы упаковываются в массив и
 * проверяются при каждом вызове — это сотни наносекунд, а на один шаг
 * обхода приходится несколько вызовов. invokeExact с точным типом
 * вызова JIT встраивает почти как прямой вызов функции C: обход из Java
 * становится в разы быстрее. Остальные (редкие) вызовы по-прежнему идут
 * через NativeCalls.invoke — там удобство важнее наносекунд.
 *
 * <p>Fast calls for the hot paths of a tree walk: navigation, the node type,
 * a child by index and a node of an XPath result. The common NativeCalls.invoke
 * goes through invokeWithArguments: the arguments are packed into an array and
 * checked on every call — that is hundreds of nanoseconds, and one walk step
 * makes several calls. The JIT inlines invokeExact with an exact call type
 * almost like a direct call of a C function: walking from Java becomes several
 * times faster. The other (rare) calls still go through NativeCalls.invoke —
 * convenience matters more than nanoseconds there.
 */
public final class NodeCalls
{
    private NodeCalls()
    {
    }

    /**
     * Узел -> узел: PARENT, FIRST_CHILD, LAST_CHILD, NEXT_SIBLING, PREVIOUS_SIBLING.
     * <p>A node -> a node: PARENT, FIRST_CHILD, LAST_CHILD, NEXT_SIBLING, PREVIOUS_SIBLING.
     */
    public static MemorySegment relative(MethodHandle function, MemorySegment node)
    {
        try
        {
            return (MemorySegment) function.invokeExact(node);
        }
        catch (Throwable error)
        {
            throw failure(error);
        }
    }

    /**
     * Узел по индексу: ребёнок (NodeFunctions.CHILD) или узел результата XPath (XPathFunctions.NODESET_NODE).
     * <p>A node by index: a child (NodeFunctions.CHILD) or a node of an XPath result (XPathFunctions.NODESET_NODE).
     */
    public static MemorySegment indexed(MethodHandle function, MemorySegment owner, long index)
    {
        try
        {
            return (MemorySegment) function.invokeExact(owner, index);
        }
        catch (Throwable error)
        {
            throw failure(error);
        }
    }

    /** Код типа узла (beresta_node_type). / The node type code (beresta_node_type). */
    public static int type(MemorySegment node)
    {
        try
        {
            return (int) NodeFunctions.TYPE.invokeExact(node);
        }
        catch (Throwable error)
        {
            throw failure(error);
        }
    }

    // Ошибки Java пробрасываются как есть, прочие — оборачиваются / Java errors pass as is, others are wrapped
    private static RuntimeException failure(Throwable error)
    {
        if (error instanceof RuntimeException runtime)
            return runtime;
        if (error instanceof Error fatal)
            throw fatal;
        return new IllegalStateException("Берёста: сбой нативного вызова / Beresta: a native call failed", error);
    }
}
