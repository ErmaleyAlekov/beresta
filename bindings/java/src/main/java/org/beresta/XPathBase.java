package org.beresta;

import java.lang.foreign.MemorySegment;

/**
 * Основа интерфейсов XPath: доступ к нативному дескриптору выражения.
 * <p>The base of the XPath interfaces: access to the native expression handle.
 */
public interface XPathBase
{
    /**
     * Дескриптор; IllegalStateException, если выражение уже закрыто.
     * <p>The handle; IllegalStateException if the expression is already closed.
     */
    MemorySegment handle();
}
