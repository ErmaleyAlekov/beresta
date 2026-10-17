using System;
using Beresta.Native;

namespace Beresta
{
    /// <summary>Удаление, копирование и фабрика обёрток.</summary>
    public partial class XmlNode
    {
        /// <summary>
        /// Удаление узла с потомками. После вызова этот объект и обёртки
        /// потомков недействительны — обращаться к ним нельзя.
        /// </summary>
        public void Remove()
        {
            NativeCalls.Check(NativeMethods.beresta_node_remove(Handle));
        }

        /// <summary>Глубокая копия в конец детей parent (можно в другой документ).</summary>
        public XmlNode CopyTo(XmlNode parent)
        {
            NativeCalls.Check(NativeMethods.beresta_node_copy_to(Handle, parent.Handle, out IntPtr copy));
            return Wrap(copy, parent.Document) ?? throw NativeCalls.LastError();
        }

        /// <summary>
        /// Фабрика обёрток (паттерн Factory Method): по типу узла выбирается
        /// класс — XmlElement, сам документ или XmlNode. NULL без ошибки — null.
        /// </summary>
        internal static XmlNode? Wrap(IntPtr pointer, XmlDocument document)
        {
            if (NativeCalls.Handle(pointer) == IntPtr.Zero)
                return null;
            if (pointer == document.Address)
                return document;
            if (NativeMethods.beresta_node_type(pointer) == (int)NodeType.Element)
                return new XmlElement(pointer, document);
            return new XmlNode(pointer, document);
        }

        /// <summary>Результат, который по контракту обязан существовать (append_*).</summary>
        private XmlNode Required(IntPtr pointer)
        {
            return Wrap(pointer, Document) ?? throw NativeCalls.LastError();
        }
    }
}
