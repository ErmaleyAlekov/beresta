using System;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Документ XML — владелец всего дерева узлов в нативной памяти.
    /// Сам является узлом и реализует IDisposable: используйте using.
    /// Если документ забыли освободить, это сделает SafeHandle при финализации.
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

        internal XmlDocument(IntPtr pointer)
            : base(pointer, null)
        {
            native = new DocumentHandle(pointer);
            Address = pointer;
        }

        /// <summary>Адрес без проверки закрытия — для фабрики обёрток.</summary>
        internal IntPtr Address { get; }

        /// <summary>true после Dispose: нативная память уже освобождена.</summary>
        public bool IsClosed
        {
            get { return native.IsClosed; }
        }

        /// <summary>Корневой элемент или null.</summary>
        public XmlElement? Root
        {
            get { return (XmlElement?)Wrap(NativeMethods.beresta_document_root(Handle), this); }
        }

        /// <summary>Освобождение нативной памяти; повторный вызов безопасен.</summary>
        public void Dispose()
        {
            native.Dispose();
        }
    }
}
