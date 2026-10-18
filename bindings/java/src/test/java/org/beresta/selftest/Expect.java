package org.beresta.selftest;

import java.util.Objects;

/**
 * Минимальные проверки для самопроверки без внешних библиотек (JUnit не нужен).
 * <p>Minimal checks for the self-test without external libraries (JUnit is not needed).
 */
final class Expect
{
    private Expect()
    {
    }

    static void equal(Object expected, Object actual, String what)
    {
        if (!Objects.equals(expected, actual))
            throw new AssertionError(what + ": ожидалось [" + expected + "], получено [" + actual + "]");
    }

    static void isTrue(boolean condition, String what)
    {
        if (!condition)
            throw new AssertionError(what);
    }

    /**
     * Вызывается после действия, которое обязано было бросить исключение.
     * <p>Called after an action that was supposed to throw an exception.
     */
    static AssertionError missingError(String what)
    {
        return new AssertionError(what + ": исключение не было брошено");
    }
}
