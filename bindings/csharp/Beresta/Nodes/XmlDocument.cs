using System;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Документ XML — владелец всего дерева узлов в нативной памяти.
    /// Сам является узлом и реализует IDisposable: используйте using.
    /// Если документ забыли освободить, это сделает SafeHandle при финализации.
    /// An XML document — the owner of the whole node tree in native memory.
    /// It is itself a node and implements IDisposable: use using.
    /// If the document is not released, SafeHandle does it on finalization.
    /// </summary>
    /// <example>
    /// <code>
    /// using XmlDocument document = Xml.Parse("&lt;shop&gt;&lt;item&gt;Чай&lt;/item&gt;&lt;/shop&gt;");
    /// Console.WriteLine(document.Root!.FirstChild!.Text);
    /// </code>
    /// </example>
    public sealed partial class XmlDocument : XmlNode, IDisposable
    {
        private readonly DocumentHandle native;

        /// <summary>Единственные обёртки узлов этого документа (см. NodeRegistry).
        /// The only wrappers of the nodes of this document (see NodeRegistry).</summary>
        internal readonly NodeRegistry Registry = new NodeRegistry();

        internal XmlDocument(IntPtr pointer)
            : base(pointer, null)
        {
            native = new DocumentHandle(pointer);
            Address = pointer;
        }

        /// <summary>Адрес без проверки закрытия — для фабрики обёрток.
        /// The address without the closed check — for the wrapper factory.</summary>
        internal IntPtr Address { get; }

        /// <summary>true после Dispose: нативная память уже освобождена.
        /// true after Dispose: the native memory is already released.</summary>
        public bool IsClosed
        {
            get { return native.IsClosed; }
        }

        /// <summary>Корневой элемент или null.
        /// The root element or null.</summary>
        public XmlElement? Root
        {
            get { return (XmlElement?)Wrap(NativeMethods.beresta_document_root(Handle), this); }
        }

        /// <summary>Освобождение нативной памяти; повторный вызов безопасен.
        /// Releases the native memory; calling it again is safe.</summary>
        public void Dispose()
        {
            native.Dispose();
        }
    }
}
