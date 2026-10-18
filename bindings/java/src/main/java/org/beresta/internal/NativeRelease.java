package org.beresta.internal;

import java.lang.foreign.MemorySegment;
import java.lang.invoke.MethodHandle;
import java.lang.ref.Cleaner;

/**
 * Освобождение нативного объекта (документа, схемы, выражения XPath).
 *
 * <p>Основной путь — явный close() (try-with-resources). Cleaner — страховка:
 * если объект забыли закрыть, нативная память всё равно освободится, когда
 * сборщик мусора уберёт объект-обёртку. Действие освобождения — отдельный
 * класс, а не лямбда: оно не должно ссылаться на саму обёртку, иначе та
 * никогда не станет недостижимой. Cleanable.clean() выполняется ровно
 * один раз — двойное освобождение невозможно.
 *
 * <p>Releases a native object (a document, a schema, an XPath expression).
 * The main path is an explicit close() (try-with-resources). Cleaner is a safety net:
 * if the object was not closed, the native memory is still freed when the garbage
 * collector removes the wrapper. The release action is a separate class, not a
 * lambda: it must not refer to the wrapper itself, or the wrapper would never
 * become unreachable. Cleanable.clean() runs exactly once — a double free is impossible.
 */
public final class NativeRelease implements Runnable
{
    private static final Cleaner CLEANER = Cleaner.create();

    private final MethodHandle freeFunction;
    private final MemorySegment handle;

    private NativeRelease(MethodHandle freeFunction, MemorySegment handle)
    {
        this.freeFunction = freeFunction;
        this.handle = handle;
    }

    /**
     * Регистрация владельца: free вызовется при close() или сборке мусора.
     * <p>Registers the owner: free is called on close() or garbage collection.
     */
    public static Cleaner.Cleanable register(Object owner, MethodHandle freeFunction, MemorySegment handle)
    {
        return CLEANER.register(owner, new NativeRelease(freeFunction, handle));
    }

    @Override
    public void run()
    {
        NativeCalls.invoke(freeFunction, handle);
    }
}
