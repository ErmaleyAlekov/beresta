using Beresta.Native;

namespace Beresta
{
    /// <summary>Навигация за O(1): ядро хранит индекс узла у родителя. Нет узла — null.
    /// O(1) navigation: the core stores the node's index in its parent. No node means null.</summary>
    public partial class XmlNode
    {
        /// <summary>Родитель (у корня — документ, у документа — null).
        /// The parent (a document for the root, null for a document).</summary>
        public XmlNode? Parent
        {
            get { return Wrap(NativeMethods.beresta_node_parent(Handle), Document); }
        }

        /// <summary>Первый ребёнок.
        /// The first child.</summary>
        public XmlNode? FirstChild
        {
            get { return Wrap(NativeMethods.beresta_node_first_child(Handle), Document); }
        }

        /// <summary>Последний ребёнок.
        /// The last child.</summary>
        public XmlNode? LastChild
        {
            get { return Wrap(NativeMethods.beresta_node_last_child(Handle), Document); }
        }

        /// <summary>Следующий сосед.
        /// The next sibling.</summary>
        public XmlNode? NextSibling
        {
            get { return Wrap(NativeMethods.beresta_node_next_sibling(Handle), Document); }
        }

        /// <summary>Предыдущий сосед.
        /// The previous sibling.</summary>
        public XmlNode? PreviousSibling
        {
            get { return Wrap(NativeMethods.beresta_node_previous_sibling(Handle), Document); }
        }
    }
}
