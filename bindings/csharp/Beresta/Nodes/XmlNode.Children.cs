using System.Collections.Generic;
using Beresta.Native;

namespace Beresta
{
    /// <summary>Доступ к детям по индексу и перечислением.</summary>
    public partial class XmlNode
    {
        /// <summary>Число детей (0 у листовых узлов).</summary>
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

        /// <summary>Ребёнок по индексу; неверный индекс — BerestaException(INVALID_ARGUMENT).</summary>
        public XmlNode Child(long index)
        {
            XmlNode? child = Wrap(NativeMethods.beresta_node_child(Handle, index), Document);

            if (child == null)
                throw NativeCalls.LastError();
            return child;
        }

        /// <summary>Перечисление детей через соседей — O(1) на шаг, без промежуточных списков.</summary>
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
