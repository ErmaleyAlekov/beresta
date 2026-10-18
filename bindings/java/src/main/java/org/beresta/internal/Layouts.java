package org.beresta.internal;

import java.lang.foreign.AddressLayout;
import java.lang.foreign.ValueLayout;

/**
 * Раскладки типов Си-интерфейса в терминах FFM API.
 * Имена повторяют типы из beresta_c.h — сигнатуры читаются как в заголовке.
 *
 * <p>Layouts of the C interface types in FFM API terms.
 * The names repeat the types of beresta_c.h — signatures read like the header.
 */
final class Layouts
{
    /**
     * Любой указатель: дескриптор, const char*, char*, выходной параметр.
     * <p>Any pointer: a handle, const char*, char*, an output parameter.
     */
    static final AddressLayout POINTER = ValueLayout.ADDRESS;

    /** int (коды состояния, типы узлов, логические 1/0/-1). / int (status codes, node types, booleans 1/0/-1). */
    static final ValueLayout.OfInt INT = ValueLayout.JAVA_INT;

    /**
     * uint32_t (флаги): в Java нет беззнаковых — те же 32 бита.
     * <p>uint32_t (flags): Java has no unsigned types — the same 32 bits.
     */
    static final ValueLayout.OfInt UINT32 = ValueLayout.JAVA_INT;

    /** int64_t (размеры и индексы). / int64_t (sizes and indexes). */
    static final ValueLayout.OfLong INT64 = ValueLayout.JAVA_LONG;

    /** double (числа XPath). / double (XPath numbers). */
    static final ValueLayout.OfDouble DOUBLE = ValueLayout.JAVA_DOUBLE;

    private Layouts()
    {
    }
}
