package org.beresta;

/**
 * Флаги вывода (BERESTA_WRITE_*). По умолчанию вывод без единого лишнего пробела.
 * <p>Output flags (BERESTA_WRITE_*). By default the output has no extra spaces at all.
 */
public final class WriteFlags
{
    public static final int DEFAULT = 0;
    public static final int PRETTY = 1;
    public static final int DECLARATION = 1 << 1;

    private WriteFlags()
    {
    }
}
