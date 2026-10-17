using System;
using System.Runtime.InteropServices;

namespace Beresta.Native
{
    /// <summary>Раздел «XPath 1.0» Си-интерфейса.</summary>
    internal static partial class NativeMethods
    {
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_xpath_compile([MarshalAs(UnmanagedType.LPUTF8Str)] string expression, out IntPtr xpath);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern void beresta_xpath_free(IntPtr xpath);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_xpath_bind_namespace(IntPtr xpath,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string prefix, [MarshalAs(UnmanagedType.LPUTF8Str)] string uri);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_xpath_set_string(IntPtr xpath,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string name, [MarshalAs(UnmanagedType.LPUTF8Str)] string value);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_xpath_set_number(IntPtr xpath, [MarshalAs(UnmanagedType.LPUTF8Str)] string name, double value);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_xpath_select(IntPtr xpath, IntPtr context, out IntPtr nodeset);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_xpath_string(IntPtr xpath, IntPtr context);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_xpath_number(IntPtr xpath, IntPtr context, out double value);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_xpath_boolean(IntPtr xpath, IntPtr context);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern long beresta_nodeset_size(IntPtr nodeset);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_nodeset_node(IntPtr nodeset, long index);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_nodeset_value(IntPtr nodeset, long index);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern void beresta_nodeset_free(IntPtr nodeset);
    }
}
