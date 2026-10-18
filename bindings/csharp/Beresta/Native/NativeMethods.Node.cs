using System;
using System.Runtime.InteropServices;

namespace Beresta.Native
{
    /// <summary>Разделы «Узлы: свойства и навигация» и «Узлы: изменение дерева».
    /// The "Nodes: properties and navigation" and "Nodes: changing the tree" sections.</summary>
    internal static partial class NativeMethods
    {
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_node_type(IntPtr node);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_name(IntPtr node);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_text(IntPtr node);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_to_string(IntPtr node, uint flags);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_parent(IntPtr node);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_first_child(IntPtr node);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_last_child(IntPtr node);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_next_sibling(IntPtr node);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_previous_sibling(IntPtr node);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern long beresta_node_child_count(IntPtr node);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_child(IntPtr node, long index);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_node_set_text(IntPtr node, [MarshalAs(UnmanagedType.LPUTF8Str)] string text);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_node_remove(IntPtr node);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_node_copy_to(IntPtr source, IntPtr parent, out IntPtr copy);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern int beresta_node_move_to(IntPtr node, IntPtr parent);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_append_element(IntPtr parent, [MarshalAs(UnmanagedType.LPUTF8Str)] string name);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_append_text(IntPtr parent, [MarshalAs(UnmanagedType.LPUTF8Str)] string text);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_append_comment(IntPtr parent, [MarshalAs(UnmanagedType.LPUTF8Str)] string text);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_append_cdata(IntPtr parent, [MarshalAs(UnmanagedType.LPUTF8Str)] string text);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        internal static extern IntPtr beresta_node_append_pi(IntPtr parent,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string target, [MarshalAs(UnmanagedType.LPUTF8Str)] string? data);
    }
}
