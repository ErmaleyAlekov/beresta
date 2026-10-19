package org.beresta;

import java.lang.foreign.MemorySegment;
import java.util.ArrayList;
import java.util.List;

import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeMemory;
import org.beresta.internal.NativeStrings;
import org.beresta.internal.SchemaFunctions;

/**
 * Чтение нативного списка нарушений в List&lt;Diagnostic&gt;. Общее для
 * проверки готового дерева и потоковой проверки байтов и файлов (DRY).
 * Список освобождается в finally — даже если чтение прервало исключение.
 *
 * <p>Reads a native list of violations into a List&lt;Diagnostic&gt;. Common for
 * the validation of a ready tree and the streaming validation of bytes and files (DRY).
 * The list is freed in finally — even if reading was interrupted by an exception.
 */
final class DiagnosticReader
{
    private DiagnosticReader()
    {
    }

    /**
     * Забирает список из выходной ячейки out и освобождает его.
     * <p>Takes the list from the output slot out and frees it.
     */
    static List<Diagnostic> take(MemorySegment out)
    {
        MemorySegment list = NativeMemory.readPointer(out);
        List<Diagnostic> result = new ArrayList<>();

        try
        {
            long count = (long) NativeCalls.invoke(SchemaFunctions.DIAGNOSTICS_COUNT, list);
            for (long index = 0; index < count; ++index)
                result.add(new Diagnostic(
                    NativeStrings.take(NativeCalls.invoke(SchemaFunctions.DIAGNOSTICS_MESSAGE, list, index)),
                    (long) NativeCalls.invoke(SchemaFunctions.DIAGNOSTICS_LINE, list, index),
                    (long) NativeCalls.invoke(SchemaFunctions.DIAGNOSTICS_COLUMN, list, index)));
            return result;
        }
        finally
        {
            NativeCalls.invoke(SchemaFunctions.DIAGNOSTICS_FREE, list);
        }
    }
}
