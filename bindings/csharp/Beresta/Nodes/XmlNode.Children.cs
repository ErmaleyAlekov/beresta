using System.Collections.Generic;
using Beresta.Native;

namespace Beresta
{
    /// <summary>Доступ к детям по индексу и перечислением.
    /// Access to children by index and by enumeration.</summary>
    public partial class XmlNode
    {
        /// <summary>Число детей (0 у листовых узлов).
        /// The number of children (0 for leaf nodes).</summary>
        public long ChildCount
        {
            get
            {
                long count = NativeMethods.beresta_node_child_count(Handle);

                if (count < 0)
                    throw NativeCalls.LastError();
                return count;
            }
        }

        /// <summary>Ребёнок по индексу; неверный индекс — BerestaException(INVALID_ARGUMENT).
        /// A child by index; an invalid index throws BerestaException(INVALID_ARGUMENT).</summary>
        public XmlNode Child(long index)
        {
            XmlNode? child = Wrap(NativeMethods.beresta_node_child(Handle, index), Document);

            if (child == null)
                throw NativeCalls.LastError();
            return child;
        }

        /// <summary>Перечисление детей через соседей — O(1) на шаг, без промежуточных списков.
        /// Enumerates children through siblings — O(1) per step, no intermediate lists.</summary>
        public IEnumerable<XmlNode> Children
        {
            get
            {
                XmlNode? child = FirstChild;

                while (child != null)
                {
                    yield return child;
                    child = child.NextSibling;
                }
            }
        }
    }
}
