using System;
using Beresta.Native;

namespace Beresta
{
    /// <summary>Удаление, копирование, перенос и фабрика обёрток.
    /// Removal, copying, moving and the wrapper factory.</summary>
    public partial class XmlNode
    {
        /// <summary>
        /// Удаление узла с потомками. После вызова этот объект и обёртки
        /// потомков недействительны — обращаться к ним нельзя.
        /// Removes the node with its descendants. After the call this object and the
        /// wrappers of its descendants are invalid — they must not be used.
        /// </summary>
        public void Remove()
        {
            NativeCalls.Check(NativeMethods.beresta_node_remove(Handle));
        }

        /// <summary>Глубокая копия в конец детей parent (можно в другой документ).
        /// A deep copy appended to the children of parent (another document is allowed).</summary>
        public XmlNode CopyTo(XmlNode parent)
        {
            NativeCalls.Check(NativeMethods.beresta_node_copy_to(Handle, parent.Handle, out IntPtr copy));
            return Wrap(copy, parent.Document) ?? throw NativeCalls.LastError();
        }

        /// <summary>
        /// Перенос (без копирования) в конец детей parent, в том числе в другой
        /// документ. Этот объект остаётся тем же узлом; после переноса его
        /// документ — parent.Document, и закрытие прежнего документа узел не
        /// затрагивает. Обёртки потомков, полученные раньше, тоже переходят
        /// к новому документу. Корень документа и сам документ переносить
        /// нельзя (BerestaException HIERARCHY_VIOLATION / INVALID_ARGUMENT).
        /// Moves the node (without copying) to the end of the children of parent,
        /// including into another document. This object stays the same node; after the
        /// move its document is parent.Document, and closing the former document does
        /// not affect the node. The wrappers of descendants obtained earlier move to the
        /// new document too. The document root and the document itself cannot be moved
        /// (BerestaException HIERARCHY_VIOLATION / INVALID_ARGUMENT).
        /// </summary>
        public void MoveTo(XmlNode parent)
        {
            XmlDocument source = Document;
            XmlDocument target = parent.Document;

            NativeCalls.Check(NativeMethods.beresta_node_move_to(Handle, parent.Handle));
            if (ReferenceEquals(source, target))
                return;
            source.Registry.Transfer(pointer, target);
            Adopt(target);
        }

        /// <summary>
        /// Фабрика обёрток (паттерн Factory Method): по типу узла выбирается
        /// класс — XmlElement, сам документ или XmlNode. NULL без ошибки — null.
        /// Уже выданная живая обёртка того же узла возвращается повторно
        /// (реестр документа, паттерн Identity Map): у одного узла одна обёртка.
        /// The wrapper factory (the Factory Method pattern): the class is chosen by the
        /// node type — XmlElement, the document itself or XmlNode. NULL without an error is null.
        /// A live wrapper already handed out for the same node is returned again
        /// (the document registry, the Identity Map pattern): one node has one wrapper.
        /// </summary>
        internal static XmlNode? Wrap(IntPtr pointer, XmlDocument document)
        {
            if (NativeCalls.Handle(pointer) == IntPtr.Zero)
                return null;
            if (pointer == document.Address)
                return document;
            bool element = NativeMethods.beresta_node_type(pointer) == (int)NodeType.Element;
            XmlNode? node = document.Registry.Find(pointer, element);
            if (node != null)
                return node;
            if (element)
                node = new XmlElement(pointer, document);
            else
                node = new XmlNode(pointer, document);
            document.Registry.Store(pointer, node);
            return node;
        }

        /// <summary>Результат, который по контракту обязан существовать (append_*).
        /// A result that by contract must exist (append_*).</summary>
        private XmlNode Required(IntPtr pointer)
        {
            return Wrap(pointer, Document) ?? throw NativeCalls.LastError();
        }
    }
}
