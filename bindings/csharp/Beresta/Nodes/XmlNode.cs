using System;

namespace Beresta
{
    /// <summary>
    /// Узел документа: нативный дескриптор плюс документ-владелец.
    ///
    /// Класс разбит на partial-файлы по обязанностям (содержимое, навигация,
    /// дети, изменение дерева) — каждый читается отдельно, как интерфейс
    /// в принципе разделения интерфейсов (ISP из SOLID).
    ///
    /// Ссылка на документ: пока жив узел, документ не будет финализирован,
    /// а после Dispose документа обращение к узлу даёт ObjectDisposedException
    /// вместо падения процесса.
    /// A document node: a native handle plus the owning document.
    /// 
    /// The class is split into partial files by responsibility (content, navigation,
    /// children, tree changes) — each is read separately, like an interface under the
    /// interface segregation principle (ISP from SOLID).
    /// 
    /// The reference to the document: while a node is alive, the document is not
    /// finalized, and after the document is disposed, accessing the node throws
    /// ObjectDisposedException instead of crashing the process.
    /// </summary>
    public partial class XmlNode
    {
        private readonly IntPtr pointer;

        // Не readonly: MoveTo передаёт узел другому документу (см. Adopt)
        // Not readonly: MoveTo hands the node over to another document (see Adopt)
        private XmlDocument? owner;

        internal XmlNode(IntPtr pointer, XmlDocument? owner)
        {
            this.pointer = pointer;
            this.owner = owner;
        }

        /// <summary>Нативный дескриптор с проверкой, что документ не закрыт.
        /// The native handle, checking that the document is not closed.</summary>
        internal IntPtr Handle
        {
            get
            {
                if (Document.IsClosed)
                    throw new ObjectDisposedException(nameof(XmlDocument), "Берёста / Beresta: " + BerestaLibrary.Message(1));
                return pointer;
            }
        }

        /// <summary>Документ, которому принадлежит узел (у документа — он сам).
        /// The document the node belongs to (for a document — itself).</summary>
        public XmlDocument Document
        {
            get
            {
                if (owner == null)
                    return (XmlDocument)this;
                return owner;
            }
        }

        /// <summary>Две обёртки равны, если указывают на один нативный узел.
        /// Two wrappers are equal if they point to the same native node.</summary>
        public override bool Equals(object? other)
        {
            return other is XmlNode node && node.pointer == pointer;
        }

        /// <inheritdoc />
        public override int GetHashCode()
        {
            return pointer.GetHashCode();
        }
    }
}
