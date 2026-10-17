using Beresta.Native;

namespace Beresta
{
    /// <summary>Навигация за O(1): ядро хранит индекс узла у родителя. Нет узла — null.</summary>
    public partial class XmlNode
    {
        /// <summary>Родитель (у корня — документ, у документа — null).</summary>
        public XmlNode? Parent
        {
            get { return Wrap(NativeMethods.beresta_node_parent(Handle), Document); }
        }

        /// <summary>Первый ребёнок.</summary>
        public XmlNode? FirstChild
        {
            get { return Wrap(NativeMethods.beresta_node_first_child(Handle), Document); }
        }

        /// <summary>Последний ребёнок.</summary>
        public XmlNode? LastChild
        {
            get { return Wrap(NativeMethods.beresta_node_last_child(Handle), Document); }
        }

        /// <summary>Следующий сосед.</summary>
        public XmlNode? NextSibling
        {
            get { return Wrap(NativeMethods.beresta_node_next_sibling(Handle), Document); }
        }

        /// <summary>Предыдущий сосед.</summary>
        public XmlNode? PreviousSibling
        {
            get { return Wrap(NativeMethods.beresta_node_previous_sibling(Handle), Document); }
        }
    }
}
