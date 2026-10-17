using System;
using System.IO;
using System.Text;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Точка входа (паттерн Facade): разбор текста, байтов и файлов,
    /// создание документа с нуля. Методы потокобезопасны.
    /// </summary>
    public static class Xml
    {
        /// <summary>Разбор строки (с необязательными флагами и проверкой по схеме).</summary>
        public static XmlDocument Parse(string xml, ParseFlags flags = ParseFlags.Default, XmlSchema? schema = null)
        {
            return Parse(Encoding.UTF8.GetBytes(xml), flags, schema);
        }

        /// <summary>
        /// Разбор байтов в любой поддерживаемой кодировке (UTF-8/16,
        /// windows-1251, KOI8-R...): её определит библиотека.
        /// </summary>
        public static XmlDocument Parse(byte[] data, ParseFlags flags = ParseFlags.Default, XmlSchema? schema = null)
        {
            int status = NativeMethods.beresta_document_parse(data, data.LongLength, (uint)flags,
                SchemaPointer(schema), out IntPtr document);

            GC.KeepAlive(schema);
            NativeCalls.Check(status);
            return new XmlDocument(document);
        }

        /// <summary>Разбор файла; ParseFlags.FileResources разрешает внешние DTD из его каталога.</summary>
        public static XmlDocument ParseFile(string path, ParseFlags flags = ParseFlags.Default, XmlSchema? schema = null)
        {
            int status = NativeMethods.beresta_document_parse_file(Path.GetFullPath(path), (uint)flags,
                SchemaPointer(schema), out IntPtr document);

            GC.KeepAlive(schema);
            NativeCalls.Check(status);
            return new XmlDocument(document);
        }

        /// <summary>Новый документ; rootName == null — без корневого элемента.</summary>
        public static XmlDocument NewDocument(string? rootName = null)
        {
            NativeCalls.Check(NativeMethods.beresta_document_create(rootName, out IntPtr document));
            return new XmlDocument(document);
        }

        /// <summary>Указатель схемы для разбора (null — без проверки по схеме).</summary>
        internal static IntPtr SchemaPointer(XmlSchema? schema)
        {
            if (schema == null)
                return IntPtr.Zero;
            if (schema.Native.IsClosed)
                throw new ObjectDisposedException(nameof(XmlSchema));
            return schema.Native.DangerousGetHandle();
        }
    }
}
