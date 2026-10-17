package org.beresta.internal;

import static org.beresta.internal.Layouts.INT;
import static org.beresta.internal.Layouts.INT64;
import static org.beresta.internal.Layouts.POINTER;
import static org.beresta.internal.Layouts.UINT32;

import java.lang.foreign.FunctionDescriptor;
import java.lang.invoke.MethodHandle;

/** Функции узлов (разделы «Узлы: свойства и навигация» и «Узлы: изменение дерева»). */
public final class NodeFunctions
{
    private static final FunctionDescriptor NODE_TO_NODE = FunctionDescriptor.of(POINTER, POINTER);
    private static final FunctionDescriptor NODE_AND_TEXT = FunctionDescriptor.of(POINTER, POINTER, POINTER);

    public static final MethodHandle TYPE = NativeLibrary.function("beresta_node_type",
        FunctionDescriptor.of(INT, POINTER));
    public static final MethodHandle NAME = NativeLibrary.function("beresta_node_name", NODE_TO_NODE);
    public static final MethodHandle TEXT = NativeLibrary.function("beresta_node_text", NODE_TO_NODE);
    public static final MethodHandle TO_STRING = NativeLibrary.function("beresta_node_to_string",
        FunctionDescriptor.of(POINTER, POINTER, UINT32));

    public static final MethodHandle PARENT = NativeLibrary.function("beresta_node_parent", NODE_TO_NODE);
    public static final MethodHandle FIRST_CHILD = NativeLibrary.function("beresta_node_first_child", NODE_TO_NODE);
    public static final MethodHandle LAST_CHILD = NativeLibrary.function("beresta_node_last_child", NODE_TO_NODE);
    public static final MethodHandle NEXT_SIBLING = NativeLibrary.function("beresta_node_next_sibling", NODE_TO_NODE);
    public static final MethodHandle PREVIOUS_SIBLING =
        NativeLibrary.function("beresta_node_previous_sibling", NODE_TO_NODE);
    public static final MethodHandle OWNER_DOCUMENT =
        NativeLibrary.function("beresta_node_owner_document", NODE_TO_NODE);
    public static final MethodHandle CHILD_COUNT = NativeLibrary.function("beresta_node_child_count",
        FunctionDescriptor.of(INT64, POINTER));
    public static final MethodHandle CHILD = NativeLibrary.function("beresta_node_child",
        FunctionDescriptor.of(POINTER, POINTER, INT64));

    public static final MethodHandle SET_TEXT = NativeLibrary.function("beresta_node_set_text",
        FunctionDescriptor.of(INT, POINTER, POINTER));
    public static final MethodHandle REMOVE = NativeLibrary.function("beresta_node_remove",
        FunctionDescriptor.of(INT, POINTER));
    public static final MethodHandle COPY_TO = NativeLibrary.function("beresta_node_copy_to",
        FunctionDescriptor.of(INT, POINTER, POINTER, POINTER));
    public static final MethodHandle APPEND_ELEMENT = NativeLibrary.function("beresta_node_append_element", NODE_AND_TEXT);
    public static final MethodHandle APPEND_TEXT = NativeLibrary.function("beresta_node_append_text", NODE_AND_TEXT);
    public static final MethodHandle APPEND_COMMENT = NativeLibrary.function("beresta_node_append_comment", NODE_AND_TEXT);
    public static final MethodHandle APPEND_CDATA = NativeLibrary.function("beresta_node_append_cdata", NODE_AND_TEXT);
    public static final MethodHandle APPEND_PI = NativeLibrary.function("beresta_node_append_pi",
        FunctionDescriptor.of(POINTER, POINTER, POINTER, POINTER));

    private NodeFunctions()
    {
    }
}
