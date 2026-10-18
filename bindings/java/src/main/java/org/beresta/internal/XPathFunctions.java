package org.beresta.internal;

import static org.beresta.internal.Layouts.DOUBLE;
import static org.beresta.internal.Layouts.INT;
import static org.beresta.internal.Layouts.INT64;
import static org.beresta.internal.Layouts.POINTER;

import java.lang.foreign.FunctionDescriptor;
import java.lang.invoke.MethodHandle;

/**
 * Функции XPath и наборов узлов (раздел «XPath 1.0» beresta_c.h).
 * <p>XPath and node-set functions (the "XPath 1.0" section of beresta_c.h).
 */
public final class XPathFunctions
{
    public static final MethodHandle COMPILE = NativeLibrary.function("beresta_xpath_compile",
        FunctionDescriptor.of(INT, POINTER, POINTER));
    public static final MethodHandle FREE = NativeLibrary.function("beresta_xpath_free",
        FunctionDescriptor.ofVoid(POINTER));
    public static final MethodHandle BIND_NAMESPACE = NativeLibrary.function("beresta_xpath_bind_namespace",
        FunctionDescriptor.of(INT, POINTER, POINTER, POINTER));
    public static final MethodHandle SET_STRING = NativeLibrary.function("beresta_xpath_set_string",
        FunctionDescriptor.of(INT, POINTER, POINTER, POINTER));
    public static final MethodHandle SET_NUMBER = NativeLibrary.function("beresta_xpath_set_number",
        FunctionDescriptor.of(INT, POINTER, POINTER, DOUBLE));
    public static final MethodHandle SELECT = NativeLibrary.function("beresta_xpath_select",
        FunctionDescriptor.of(INT, POINTER, POINTER, POINTER));
    public static final MethodHandle STRING = NativeLibrary.function("beresta_xpath_string",
        FunctionDescriptor.of(POINTER, POINTER, POINTER));
    public static final MethodHandle NUMBER = NativeLibrary.function("beresta_xpath_number",
        FunctionDescriptor.of(INT, POINTER, POINTER, POINTER));
    public static final MethodHandle BOOLEAN = NativeLibrary.function("beresta_xpath_boolean",
        FunctionDescriptor.of(INT, POINTER, POINTER));

    public static final MethodHandle NODESET_SIZE = NativeLibrary.function("beresta_nodeset_size",
        FunctionDescriptor.of(INT64, POINTER));
    public static final MethodHandle NODESET_NODE = NativeLibrary.function("beresta_nodeset_node",
        FunctionDescriptor.of(POINTER, POINTER, INT64));
    public static final MethodHandle NODESET_VALUE = NativeLibrary.function("beresta_nodeset_value",
        FunctionDescriptor.of(POINTER, POINTER, INT64));
    public static final MethodHandle NODESET_FREE = NativeLibrary.function("beresta_nodeset_free",
        FunctionDescriptor.ofVoid(POINTER));

    private XPathFunctions()
    {
    }
}
