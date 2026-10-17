package org.beresta.internal;

import java.lang.foreign.AddressLayout;
import java.lang.foreign.ValueLayout;

/**
 * Раскладки типов Си-интерфейса в терминах FFM API.
 * Имена повторяют типы из beresta_c.h — сигнатуры читаются как в заголовке.
 */
final class Layouts
{
    /** Любой указатель: дескриптор, const char*, char*, выходной параметр. */
    static final AddressLayout POINTER = ValueLayout.ADDRESS;

    /** int (коды состояния, типы узлов, логические 1/0/-1). */
    static final ValueLayout.OfInt INT = ValueLayout.JAVA_INT;

    /** uint32_t (флаги): в Java нет беззнаковых — те же 32 бита. */
    static final ValueLayout.OfInt UINT32 = ValueLayout.JAVA_INT;

    /** int64_t (размеры и индексы). */
    static final ValueLayout.OfLong INT64 = ValueLayout.JAVA_LONG;

    /** double (числа XPath). */
    static final ValueLayout.OfDouble DOUBLE = ValueLayout.JAVA_DOUBLE;

    private Layouts()
    {
    }
}
