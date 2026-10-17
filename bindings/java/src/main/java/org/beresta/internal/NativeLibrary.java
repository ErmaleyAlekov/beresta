package org.beresta.internal;

import java.lang.foreign.FunctionDescriptor;
import java.lang.foreign.Linker;
import java.lang.foreign.SymbolLookup;
import java.lang.invoke.MethodHandle;

/**
 * Связь с нативной библиотекой через Project Panama (FFM API, Java 22+).
 *
 * <p>Вместо JNI здесь нет ни строчки кода на C: Linker по описанию
 * сигнатуры (FunctionDescriptor) сам строит вызов функции Си-интерфейса.
 * Библиотека загружается один раз при первом обращении к классу
 * (ленивая инициализация держателя — потокобезопасна по правилам JVM).
 */
public final class NativeLibrary
{
    private static final SymbolLookup LOOKUP = NativeLoader.load();
    private static final Linker LINKER = Linker.nativeLinker();

    private NativeLibrary()
    {
    }

    /** Дескриптор вызова Си-функции по имени и сигнатуре. */
    public static MethodHandle function(String name, FunctionDescriptor descriptor)
    {
        return LINKER.downcallHandle(LOOKUP.find(name).orElseThrow(), descriptor);
    }
}
