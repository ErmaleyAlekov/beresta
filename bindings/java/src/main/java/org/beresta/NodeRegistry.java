package org.beresta;

import java.lang.foreign.MemorySegment;
import java.lang.ref.WeakReference;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NodeFunctions;

/**
 * Реестр обёрток одного документа: нативный адрес узла → его единственная
 * обёртка (паттерн Identity Map). Зачем он нужен: при переносе узла в другой
 * документ (moveTo) у ВСЕХ уже выданных обёрток этого узла и его потомков
 * должен смениться документ-владелец. Иначе обёртка, полученная до переноса,
 * продолжала бы проверять «жив ли документ» по старому документу: после
 * закрытия нового она обратилась бы к освобождённой памяти и уронила бы JVM.
 *
 * <p>Реестр хранит слабые ссылки и не продлевает жизнь обёрткам; записи об
 * умерших обёртках вычищаются, когда реестр вырастает вдвое. Запись об
 * удалённом узле может пережить его: адрес достанется новому узлу, поэтому
 * при поиске сверяется вид обёртки (элемент или нет). synchronized защищает
 * только сам словарь — правило «один документ — один поток за раз» в силе.
 *
 * <p>The wrapper registry of one document: the native node address → its only
 * wrapper (the Identity Map pattern). Why it is needed: when a node is moved into
 * another document (moveTo), ALL wrappers already handed out for this node and its
 * descendants must change their owner document. Otherwise a wrapper obtained before
 * the move would keep checking "is the document alive" against the old document:
 * after the new one is closed, it would access freed memory and crash the JVM.
 *
 * <p>The registry keeps weak references and does not extend the life of wrappers;
 * the entries of dead wrappers are swept out when the registry doubles. The entry of
 * a removed node may outlive it: the address goes to a new node, so the lookup
 * compares the wrapper kind (an element or not). synchronized only protects the map
 * itself — the rule "one document — one thread at a time" still holds.
 */
final class NodeRegistry
{
    // Минимальный размер, с которого начинается чистка / The minimal size the sweep starts from
    private static final int MINIMAL_SWEEP_SIZE = 64;

    private final Map<Long, WeakReference<XmlNode>> wrappers = new HashMap<>();
    private int sweepAt = MINIMAL_SWEEP_SIZE;

    /**
     * Живая обёртка узла нужного вида или null.
     * <p>A live wrapper of the node of the required kind or null.
     */
    synchronized XmlNode find(long address, boolean element)
    {
        WeakReference<XmlNode> reference = wrappers.get(address);

        if (reference == null)
            return null;
        XmlNode node = reference.get();
        if (node == null || (node instanceof XmlElement) != element)
            return null;
        return node;
    }

    /**
     * Запоминание обёртки. Когда словарь вырос до порога, из него удаляются
     * записи умерших обёрток, а порог становится вдвое больше оставшегося:
     * чистка редкая, в среднем O(1) на вставку.
     *
     * <p>Remembers a wrapper. When the map has grown to the threshold, the entries
     * of dead wrappers are removed, and the threshold becomes twice the rest: the
     * sweep is rare, O(1) per insertion on average.
     */
    synchronized void store(long address, XmlNode node)
    {
        wrappers.put(address, new WeakReference<>(node));
        if (wrappers.size() < sweepAt)
            return;
        List<Long> dead = new ArrayList<>();
        for (Map.Entry<Long, WeakReference<XmlNode>> entry : wrappers.entrySet())
        {
            if (entry.getValue().get() == null)
                dead.add(entry.getKey());
        }
        for (Long key : dead)
            wrappers.remove(key);
        sweepAt = Math.max(MINIMAL_SWEEP_SIZE, wrappers.size() * 2);
    }

    /**
     * Извлечение живой обёртки из реестра (null — её нет).
     * <p>Takes a live wrapper out of the registry (null — there is none).
     */
    private synchronized XmlNode take(long address)
    {
        WeakReference<XmlNode> reference = wrappers.remove(address);

        if (reference == null)
            return null;
        return reference.get();
    }

    /**
     * Передача обёрток перенесённого поддерева документу target (и самой
     * обёртки moved, даже если её нет в реестре). Узлы уже живут в target,
     * поэтому обход нативного поддерева безопасен. Обход итеративный (явный
     * стек): глубина дерева не ограничена стеком вызовов. Если в реестре нет
     * никого, кроме корня поддерева, обходить потомков незачем.
     *
     * <p>Hands the wrappers of the moved subtree over to the target document (and the
     * moved wrapper itself, even if it is not in the registry). The nodes already live
     * in target, so walking the native subtree is safe. The walk is iterative (an
     * explicit stack): the tree depth is not limited by the call stack. If the registry
     * holds nobody but the subtree root, there is no need to walk the descendants.
     */
    void transfer(XmlNode moved, MemorySegment root, XmlDocument target)
    {
        boolean deep;
        ArrayDeque<MemorySegment> pending = new ArrayDeque<>();

        synchronized (this)
        {
            deep = wrappers.size() > 1;
        }
        moved.owner = target;
        pending.push(root);
        while (!pending.isEmpty())
        {
            MemorySegment current = pending.pop();
            XmlNode node = take(current.address());

            if (node != null)
            {
                node.owner = target;
                target.registry.store(current.address(), node);
            }
            for (MemorySegment child = NativeCalls.handle(NativeCalls.invoke(NodeFunctions.FIRST_CHILD, current));
                 deep && child != null;
                 child = NativeCalls.handle(NativeCalls.invoke(NodeFunctions.NEXT_SIBLING, child)))
                pending.push(child);
        }
    }
}
