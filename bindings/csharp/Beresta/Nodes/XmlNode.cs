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
    /// </summary>
    public partial class XmlNode
    {
        private readonly IntPtr pointer;
        private readonly XmlDocument? owner;

        internal XmlNode(IntPtr pointer, XmlDocument? owner)
        {
            this.pointer = pointer;
            this.owner = owner;
        }

        /// <summary>Нативный дескриптор с проверкой, что документ не закрыт.</summary>
        internal IntPtr Handle
        {
            get
            {
                if (Document.IsClosed)
                    throw new ObjectDisposedException(nameof(XmlDocument), "Берёста: документ уже закрыт, его узлы недействительны");
                return pointer;
            }
        }

        /// <summary>Документ, которому принадлежит узел (у документа — он сам).</summary>
        public XmlDocument Document
        {
            get
            {
                if (owner == null)
                    return (XmlDocument)this;
                return owner;
            }
        }

        /// <summary>Две обёртки равны, если указывают на один нативный узел.</summary>
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
