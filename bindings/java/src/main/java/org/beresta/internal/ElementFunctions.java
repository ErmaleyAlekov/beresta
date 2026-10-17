package org.beresta.internal;

import static org.beresta.internal.Layouts.INT;
import static org.beresta.internal.Layouts.INT64;
import static org.beresta.internal.Layouts.POINTER;

import java.lang.foreign.FunctionDescriptor;
import java.lang.invoke.MethodHandle;

/** Функции элементов и атрибутов (раздел «Элементы и атрибуты» beresta_c.h). */
public final class ElementFunctions
{
    public static final MethodHandle GET_ATTRIBUTE = NativeLibrary.function("beresta_element_get_attribute",
        FunctionDescriptor.of(POINTER, POINTER, POINTER));
    public static final MethodHandle SET_ATTRIBUTE = NativeLibrary.function("beresta_element_set_attribute",
        FunctionDescriptor.of(INT, POINTER, POINTER, POINTER));
    public static final MethodHandle HAS_ATTRIBUTE = NativeLibrary.function("beresta_element_has_attribute",
        FunctionDescriptor.of(INT, POINTER, POINTER));
    public static final MethodHandle REMOVE_ATTRIBUTE = NativeLibrary.function("beresta_element_remove_attribute",
        FunctionDescriptor.of(INT, POINTER, POINTER));
    public static final MethodHandle ATTRIBUTE_COUNT = NativeLibrary.function("beresta_element_attribute_count",
        FunctionDescriptor.of(INT64, POINTER));
    public static final MethodHandle ATTRIBUTE_NAME = NativeLibrary.function("beresta_element_attribute_name",
        FunctionDescriptor.of(POINTER, POINTER, INT64));
    public static final MethodHandle ATTRIBUTE_VALUE = NativeLibrary.function("beresta_element_attribute_value",
        FunctionDescriptor.of(POINTER, POINTER, INT64));
    public static final MethodHandle LOCAL_NAME = NativeLibrary.function("beresta_element_local_name",
        FunctionDescriptor.of(POINTER, POINTER));
    public static final MethodHandle NAMESPACE_URI = NativeLibrary.function("beresta_element_namespace_uri",
        FunctionDescriptor.of(POINTER, POINTER));

    private ElementFunctions()
    {
    }
}
