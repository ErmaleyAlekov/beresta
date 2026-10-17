package org.beresta;

import org.beresta.internal.LibraryFunctions;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeLoader;
import org.beresta.internal.NativeStrings;

/** Сведения о библиотеке. */
public final class Beresta
{
    private Beresta()
    {
    }

    /** Версия нативной библиотеки, например "2.1.0". */
    public static String version()
    {
        return NativeStrings.borrowed(NativeCalls.invoke(LibraryFunctions.VERSION));
    }

    /** Платформа, для которой ищется бинарник в ресурсах: windows-x86_64 и т.п. */
    public static String platform()
    {
        return NativeLoader.platformName();
    }
}
