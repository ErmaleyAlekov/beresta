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
    /// </summary>
    public sealed class XmlSchema : IDisposable
    {
        private XmlSchema(IntPtr pointer)
        {
            Native = new SchemaHandle(pointer);
        }

        /// <summary>Нативный дескриптор (SafeHandle) схемы.</summary>
        internal SchemaHandle Native { get; }

        /// <summary>Схема из текста XSD (xs:include здесь недоступен — нет каталога).</summary>
        public static XmlSchema FromString(string xsd)
        {
            byte[] data = Encoding.UTF8.GetBytes(xsd);

            NativeCalls.Check(NativeMethods.beresta_schema_load(data, data.LongLength, out IntPtr schema));
            return new XmlSchema(schema);
        }

        /// <summary>Схема из файла; xs:include / xs:import ищутся в его каталоге.</summary>
        public static XmlSchema FromFile(string path)
        {
            NativeCalls.Check(NativeMethods.beresta_schema_load_file(Path.GetFullPath(path), out IntPtr schema));
            return new XmlSchema(schema);
        }

        /// <summary>Все нарушения за один проход; пустой список — документ корректен.</summary>
        public IReadOnlyList<Diagnostic> Validate(XmlDocument document)
        {
            int status = NativeMethods.beresta_schema_validate(Xml.SchemaPointer(this), document.Handle, out IntPtr list);

            GC.KeepAlive(this);
            NativeCalls.Check(status);
            return Diagnostic.ReadAll(list);
        }

        /// <summary>Освобождение нативной памяти схемы.</summary>
        public void Dispose()
        {
            Native.Dispose();
        }
    }
}
