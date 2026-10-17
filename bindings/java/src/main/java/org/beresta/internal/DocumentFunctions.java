package org.beresta.internal;

import static org.beresta.internal.Layouts.INT;
import static org.beresta.internal.Layouts.INT64;
import static org.beresta.internal.Layouts.POINTER;
import static org.beresta.internal.Layouts.UINT32;

import java.lang.foreign.FunctionDescriptor;
import java.lang.invoke.MethodHandle;

/** Функции документов (раздел «Документы» beresta_c.h). */
public final class DocumentFunctions
{
    public static final MethodHandle CREATE = NativeLibrary.function("beresta_document_create",
        FunctionDescriptor.of(INT, POINTER, POINTER));
    public static final MethodHandle PARSE = NativeLibrary.function("beresta_document_parse",
        FunctionDescriptor.of(INT, POINTER, INT64, UINT32, POINTER, POINTER));
    public static final MethodHandle PARSE_FILE = NativeLibrary.function("beresta_document_parse_file",
        FunctionDescriptor.of(INT, POINTER, UINT32, POINTER, POINTER));
    public static final MethodHandle FREE = NativeLibrary.function("beresta_document_free",
        FunctionDescriptor.ofVoid(POINTER));
    public static final MethodHandle CLONE = NativeLibrary.function("beresta_document_clone",
        FunctionDescriptor.of(INT, POINTER, POINTER));
    public static final MethodHandle ROOT = NativeLibrary.function("beresta_document_root",
        FunctionDescriptor.of(POINTER, POINTER));
    public static final MethodHandle ELEMENT_BY_ID = NativeLibrary.function("beresta_document_element_by_id",
        FunctionDescriptor.of(POINTER, POINTER, POINTER));
    public static final MethodHandle SAVE = NativeLibrary.function("beresta_document_save",
        FunctionDescriptor.of(INT, POINTER, POINTER, UINT32, POINTER));

    private DocumentFunctions()
    {
    }
}
