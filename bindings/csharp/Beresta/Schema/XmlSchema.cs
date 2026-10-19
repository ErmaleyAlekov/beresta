using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Загруженная XML Schema. После загрузки неизменяема: одну схему можно
    /// использовать для множества документов, в том числе из разных потоков.
    /// Потоковая проверка без дерева — в XmlSchema.Streaming.cs.
    /// A loaded XML Schema. Immutable after loading: one schema can be used
    /// for many documents, including from different threads.
    /// Streaming validation without a tree is in XmlSchema.Streaming.cs.
    /// </summary>
    public sealed partial class XmlSchema : IDisposable
    {
        private XmlSchema(IntPtr pointer)
        {
            Native = new SchemaHandle(pointer);
        }

        /// <summary>Нативный дескриптор (SafeHandle) схемы.
        /// The native handle (SafeHandle) of the schema.</summary>
        internal SchemaHandle Native { get; }

        /// <summary>Схема из текста XSD (xs:include здесь недоступен — нет каталога).
        /// A schema from XSD text (xs:include is unavailable here — there is no directory).</summary>
        public static XmlSchema FromString(string xsd)
        {
            byte[] data = Encoding.UTF8.GetBytes(xsd);

            NativeCalls.Check(NativeMethods.beresta_schema_load(data, data.LongLength, out IntPtr schema));
            return new XmlSchema(schema);
        }

        /// <summary>Схема из файла; xs:include / xs:import ищутся в его каталоге.
        /// A schema from a file; xs:include / xs:import are looked up in its directory.</summary>
        public static XmlSchema FromFile(string path)
        {
            NativeCalls.Check(NativeMethods.beresta_schema_load_file(Path.GetFullPath(path), out IntPtr schema));
            return new XmlSchema(schema);
        }

        /// <summary>Все нарушения за один проход; пустой список — документ корректен.
        /// All violations in one pass; an empty list means the document is valid.</summary>
        public IReadOnlyList<Diagnostic> Validate(XmlDocument document)
        {
            int status = NativeMethods.beresta_schema_validate(Xml.SchemaPointer(this), document.Handle, out IntPtr list);

            GC.KeepAlive(this);
            NativeCalls.Check(status);
            return Diagnostic.ReadAll(list);
        }

        /// <summary>Освобождение нативной памяти схемы.
        /// Releases the native memory of the schema.</summary>
        public void Dispose()
        {
            Native.Dispose();
        }
    }
}
