using System;
using System.IO;
using Beresta.Native;

namespace Beresta
{
    /// <summary>Операции уровня документа: поиск по ID, копия, запись в файл.
    /// Document-level operations: lookup by ID, copying, writing to a file.</summary>
    public sealed partial class XmlDocument
    {
        /// <summary>Элемент с атрибутом типа ID (по DTD) или xml:id; null — не найден.
        /// An element with an ID-typed attribute (by DTD) or xml:id; null if not found.</summary>
        public XmlElement? ElementById(string id)
        {
            return (XmlElement?)Wrap(NativeMethods.beresta_document_element_by_id(Handle, id), this);
        }

        /// <summary>Независимая глубокая копия; её тоже нужно освободить.
        /// An independent deep copy; it must be released too.</summary>
        public XmlDocument Copy()
        {
            NativeCalls.Check(NativeMethods.beresta_document_clone(Handle, out IntPtr copy));
            return new XmlDocument(copy);
        }

        /// <summary>Запись в файл; encoding == null — UTF-8, иначе, например, "windows-1251".
        /// Writes to a file; encoding == null means UTF-8, otherwise, for example, "windows-1251".</summary>
        public void Save(string path, WriteFlags flags = WriteFlags.Default, string? encoding = null)
        {
            NativeCalls.Check(NativeMethods.beresta_document_save(Handle, Path.GetFullPath(path), (uint)flags, encoding));
        }
    }
}
