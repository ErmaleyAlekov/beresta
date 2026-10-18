using System;
using System.Collections.Generic;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Реестр обёрток одного документа: нативный адрес узла → его единственная
    /// обёртка (паттерн Identity Map). Зачем он нужен: при переносе узла
    /// в другой документ (MoveTo) у ВСЕХ уже выданных обёрток этого узла и
    /// его потомков должен смениться документ-владелец. Иначе обёртка,
    /// полученная до переноса, продолжала бы проверять «жив ли документ»
    /// по старому документу: после закрытия нового она обратилась бы
    /// к освобождённой памяти и уронила бы процесс.
    ///
    /// Реестр хранит слабые ссылки: он не продлевает жизнь обёрткам, а
    /// записи об умерших обёртках вычищаются, когда реестр вырастает вдвое.
    /// Записи об удалённых узлах не вычищаются сразу: адрес может достаться
    /// новому узлу, поэтому при поиске сверяется вид обёртки (элемент или нет).
    /// Блокировка нужна только для защиты самого словаря: правило «один
    /// документ — один поток за раз» остаётся в силе.
    ///
    /// The wrapper registry of one document: the native node address → its only
    /// wrapper (the Identity Map pattern). Why it is needed: when a node is moved
    /// into another document (MoveTo), ALL wrappers already handed out for this node
    /// and its descendants must change their owner document. Otherwise a wrapper
    /// obtained before the move would keep checking "is the document alive" against
    /// the old document: after the new one is closed, it would access freed memory
    /// and crash the process.
    ///
    /// The registry keeps weak references: it does not extend the life of wrappers,
    /// and the entries of dead wrappers are swept out when the registry doubles.
    /// The entries of removed nodes are not swept at once: the address may go to a
    /// new node, so the lookup compares the wrapper kind (an element or not).
    /// The lock only protects the dictionary itself: the rule "one document — one
    /// thread at a time" still holds.
    /// </summary>
    internal sealed class NodeRegistry
    {
        // Минимальный размер, с которого начинается чистка / The minimal size the sweep starts from
        private const int MinimalSweepSize = 64;

        private readonly Dictionary<IntPtr, WeakReference<XmlNode>> wrappers = new Dictionary<IntPtr, WeakReference<XmlNode>>();
        private int sweepAt = MinimalSweepSize;

        /// <summary>Живая обёртка узла нужного вида или null.
        /// A live wrapper of the node of the required kind or null.</summary>
        internal XmlNode? Find(IntPtr pointer, bool element)
        {
            lock (wrappers)
            {
                if (!wrappers.TryGetValue(pointer, out WeakReference<XmlNode>? reference))
                    return null;
                if (!reference.TryGetTarget(out XmlNode? node))
                    return null;
                if ((node is XmlElement) != element)
                    return null;
                return node;
            }
        }

        /// <summary>
        /// Запоминание обёртки. Когда словарь вырос до порога, из него
        /// удаляются записи умерших обёрток, а порог становится вдвое больше
        /// оставшегося: чистка выполняется редко, в среднем O(1) на вставку.
        /// Remembers a wrapper. When the dictionary has grown to the threshold, the
        /// entries of dead wrappers are removed from it, and the threshold becomes
        /// twice the rest: the sweep is rare, O(1) per insertion on average.
        /// </summary>
        internal void Store(IntPtr pointer, XmlNode node)
        {
            lock (wrappers)
            {
                wrappers[pointer] = new WeakReference<XmlNode>(node);
                if (wrappers.Count < sweepAt)
                    return;
                List<IntPtr> dead = new List<IntPtr>();
                foreach (KeyValuePair<IntPtr, WeakReference<XmlNode>> entry in wrappers)
                {
                    if (!entry.Value.TryGetTarget(out _))
                        dead.Add(entry.Key);
                }
                foreach (IntPtr key in dead)
                    wrappers.Remove(key);
                sweepAt = Math.Max(MinimalSweepSize, wrappers.Count * 2);
            }
        }

        /// <summary>Извлечение живой обёртки из реестра (null — её нет).
        /// Takes a live wrapper out of the registry (null — there is none).</summary>
        private XmlNode? Take(IntPtr pointer)
        {
            lock (wrappers)
            {
                if (!wrappers.Remove(pointer, out WeakReference<XmlNode>? reference))
                    return null;
                if (!reference.TryGetTarget(out XmlNode? node))
                    return null;
                return node;
            }
        }

        /// <summary>
        /// Передача обёрток перенесённого поддерева документу target. Узлы
        /// уже живут в target, поэтому обход нативного поддерева безопасен.
        /// Обход итеративный (явный стек): глубина дерева не ограничена стеком
        /// вызовов. Если в реестре нет никого, кроме корня поддерева, обходить
        /// потомков незачем — их обёрток не существует.
        /// Hands the wrappers of the moved subtree over to the target document. The
        /// nodes already live in target, so walking the native subtree is safe. The
        /// walk is iterative (an explicit stack): the tree depth is not limited by the
        /// call stack. If the registry holds nobody but the subtree root, there is no
        /// need to walk the descendants — their wrappers do not exist.
        /// </summary>
        internal void Transfer(IntPtr root, XmlDocument target)
        {
            bool deep;
            Stack<IntPtr> pending = new Stack<IntPtr>();

            lock (wrappers)
                deep = wrappers.Count > 1;

            pending.Push(root);
            while (pending.Count > 0)
            {
                IntPtr current = pending.Pop();
                XmlNode? node = Take(current);

                if (node != null)
                {
                    node.Adopt(target);
                    target.Registry.Store(current, node);
                }
                if (!deep)
                    continue;
                for (IntPtr child = NativeMethods.beresta_node_first_child(current); child != IntPtr.Zero;
                     child = NativeMethods.beresta_node_next_sibling(child))
                    pending.Push(child);
            }
        }
    }
}
