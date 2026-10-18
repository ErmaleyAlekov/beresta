using System;
using System.Runtime.InteropServices;

namespace Beresta.Native
{
    /// <summary>Раздел «Элементы и атрибуты» Си-интерфейса.
    /// The "Elements and attributes" section of the C interface.</summary>
    internal static partial class NativeMethods
    {
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_element_get_attribute(IntPtr element, [MarshalAs(UnmanagedType.LPUTF8Str)] string name);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_element_set_attribute(IntPtr element,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string name, [MarshalAs(UnmanagedType.LPUTF8Str)] string value);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_element_has_attribute(IntPtr element, [MarshalAs(UnmanagedType.LPUTF8Str)] string name);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_element_remove_attribute(IntPtr element, [MarshalAs(UnmanagedType.LPUTF8Str)] string name);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern long beresta_element_attribute_count(IntPtr element);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_element_attribute_name(IntPtr element, long index);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_element_attribute_value(IntPtr element, long index);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_element_local_name(IntPtr element);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_element_namespace_uri(IntPtr element);
    }
}
