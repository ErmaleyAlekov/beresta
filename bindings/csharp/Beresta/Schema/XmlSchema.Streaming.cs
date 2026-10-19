using System;
using System.Collections.Generic;
using System.IO;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Потоковая проверка по схеме без построения дерева (SAX + XSD внутри
    /// ядра). Память — O(глубины документа) плюс значения ID и таблицы ключей;
    /// файл отображается в память, а не копируется. Так проверяются выгрузки
    /// в гигабайты, которые в дерево не помещаются. Нарушения схемы
    /// возвращаются списком, синтаксическая ошибка XML — BerestaException.
    ///
    /// Streaming schema validation without building a tree (SAX + XSD inside
    /// the core). The memory is O(document depth) plus ID values and key tables;
    /// the file is mapped into memory, not copied. This is how exports of
    /// gigabytes that do not fit into a tree are validated. Schema violations
    /// are returned as a list, an XML syntax error is a BerestaException.
    /// </summary>
    public sealed partial class XmlSchema
    {
        /// <summary>Потоковая проверка байтов в любой поддерживаемой кодировке.
        /// Streaming validation of bytes in any supported encoding.</summary>
        public IReadOnlyList<Diagnostic> Validate(byte[] data, ParseFlags flags = ParseFlags.Default)
        {
            int status = NativeMethods.beresta_schema_validate_bytes(data, data.LongLength, (uint)flags,
                Xml.SchemaPointer(this), out IntPtr list);

            GC.KeepAlive(this);
            NativeCalls.Check(status);
            return Diagnostic.ReadAll(list);
        }

        /// <summary>Потоковая проверка файла; ParseFlags.FileResources разрешает внешние DTD из его каталога.
        /// Streaming validation of a file; ParseFlags.FileResources allows external DTDs from its directory.</summary>
        public IReadOnlyList<Diagnostic> ValidateFile(string path, ParseFlags flags = ParseFlags.Default)
        {
            int status = NativeMethods.beresta_schema_validate_file(Path.GetFullPath(path), (uint)flags,
                Xml.SchemaPointer(this), out IntPtr list);

            GC.KeepAlive(this);
            NativeCalls.Check(status);
            return Diagnostic.ReadAll(list);
        }
    }
}
