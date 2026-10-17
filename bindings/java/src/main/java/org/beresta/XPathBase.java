package org.beresta;

import java.lang.foreign.MemorySegment;

/** Основа интерфейсов XPath: доступ к нативному дескриптору выражения. */
public interface XPathBase
{
    /** Дескриптор; IllegalStateException, если выражение уже закрыто. */
    MemorySegment handle();
}
