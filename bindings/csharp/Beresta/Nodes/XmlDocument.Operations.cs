using System;
using System.IO;
using Beresta.Native;

namespace Beresta
{
    /// <summary>Операции уровня документа: поиск по ID, копия, запись в файл.</summary>
    public sealed partial class XmlDocument
    {
        /// <summary>Элемент с атрибутом типа ID (по DTD) или xml:id; null — не найден.</summary>
        public XmlElement? ElementById(string id)
        {
            return (XmlElement?)Wrap(NativeMethods.beresta_document_element_by_id(Handle, id), this);
        }

        /// <summary>Независимая глубокая копия; её тоже нужно освободить.</summary>
        public XmlDocument Copy()
        {
            NativeCalls.Check(NativeMethods.beresta_document_clone(Handle, out IntPtr copy));
            return new XmlDocument(copy);
        }

        /// <summary>Запись в файл; encoding == null — UTF-8, иначе, например, "windows-1251".</summary>
        public void Save(string path, WriteFlags flags = WriteFlags.Default, string? encoding = null)
        {
            NativeCalls.Check(NativeMethods.beresta_document_save(Handle, Path.GetFullPath(path), (uint)flags, encoding));
        }
    }
}
