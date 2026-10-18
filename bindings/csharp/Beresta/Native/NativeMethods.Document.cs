using System;
using System.Runtime.InteropServices;

namespace Beresta.Native
{
    /// <summary>Раздел «Документы» Си-интерфейса.
    /// The "Documents" section of the C interface.</summary>
    internal static partial class NativeMethods
    {
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_document_create(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string? rootName, out IntPtr document);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_document_parse(byte[] data, long size, uint flags,
            IntPtr schema, out IntPtr document);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_document_parse_file([MarshalAs(UnmanagedType.LPUTF8Str)] string path,
            uint flags, IntPtr schema, out IntPtr document);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern void beresta_document_free(IntPtr document);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_document_clone(IntPtr document, out IntPtr copy);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_document_root(IntPtr document);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_document_element_by_id(IntPtr document,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string id);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_document_save(IntPtr document,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string path, uint flags,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string? encoding);
    }
}
