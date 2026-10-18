using System;
using System.Runtime.InteropServices;

namespace Beresta.Native
{
    /// <summary>Раздел «XML Schema» Си-интерфейса.
    /// The "XML Schema" section of the C interface.</summary>
    internal static partial class NativeMethods
    {
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_schema_load(byte[] data, long size, out IntPtr schema);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_schema_load_file([MarshalAs(UnmanagedType.LPUTF8Str)] string path, out IntPtr schema);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern void beresta_schema_free(IntPtr schema);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_schema_validate(IntPtr schema, IntPtr document, out IntPtr diagnostics);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern long beresta_diagnostics_count(IntPtr diagnostics);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_diagnostics_message(IntPtr diagnostics, long index);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern long beresta_diagnostics_line(IntPtr diagnostics, long index);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern long beresta_diagnostics_column(IntPtr diagnostics, long index);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern void beresta_diagnostics_free(IntPtr diagnostics);
    }
}
