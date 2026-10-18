package org.beresta;

import org.beresta.internal.LibraryFunctions;
import org.beresta.internal.NativeCalls;
import org.beresta.internal.NativeLoader;
import org.beresta.internal.NativeStrings;

/** Сведения о библиотеке. / Library information. */
public final class Beresta
{
    private Beresta()
    {
    }

    /** Версия нативной библиотеки, например "2.1.0". / The native library version, for example "2.1.0". */
    public static String version()
    {
        return NativeStrings.borrowed(NativeCalls.invoke(LibraryFunctions.VERSION));
    }

    /**
     * Платформа, для которой ищется бинарник в ресурсах: windows-x86_64 и т.п.
     * <p>The platform whose binary is looked up in the resources: windows-x86_64 and so on.
     */
    public static String platform()
    {
        return NativeLoader.platformName();
    }
}
