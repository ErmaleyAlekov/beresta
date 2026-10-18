package org.beresta.internal;

import static org.beresta.internal.Layouts.INT;
import static org.beresta.internal.Layouts.INT64;
import static org.beresta.internal.Layouts.POINTER;

import java.lang.foreign.FunctionDescriptor;
import java.lang.invoke.MethodHandle;

/**
 * Функции XML Schema и диагностик (раздел «XML Schema» beresta_c.h).
 * <p>XML Schema and diagnostics functions (the "XML Schema" section of beresta_c.h).
 */
public final class SchemaFunctions
{
    public static final MethodHandle LOAD = NativeLibrary.function("beresta_schema_load",
        FunctionDescriptor.of(INT, POINTER, INT64, POINTER));
    public static final MethodHandle LOAD_FILE = NativeLibrary.function("beresta_schema_load_file",
        FunctionDescriptor.of(INT, POINTER, POINTER));
    public static final MethodHandle FREE = NativeLibrary.function("beresta_schema_free",
        FunctionDescriptor.ofVoid(POINTER));
    public static final MethodHandle VALIDATE = NativeLibrary.function("beresta_schema_validate",
        FunctionDescriptor.of(INT, POINTER, POINTER, POINTER));

    public static final MethodHandle DIAGNOSTICS_COUNT = NativeLibrary.function("beresta_diagnostics_count",
        FunctionDescriptor.of(INT64, POINTER));
    public static final MethodHandle DIAGNOSTICS_MESSAGE = NativeLibrary.function("beresta_diagnostics_message",
        FunctionDescriptor.of(POINTER, POINTER, INT64));
    public static final MethodHandle DIAGNOSTICS_LINE = NativeLibrary.function("beresta_diagnostics_line",
        FunctionDescriptor.of(INT64, POINTER, INT64));
    public static final MethodHandle DIAGNOSTICS_COLUMN = NativeLibrary.function("beresta_diagnostics_column",
        FunctionDescriptor.of(INT64, POINTER, INT64));
    public static final MethodHandle DIAGNOSTICS_FREE = NativeLibrary.function("beresta_diagnostics_free",
        FunctionDescriptor.ofVoid(POINTER));

    private SchemaFunctions()
    {
    }
}
