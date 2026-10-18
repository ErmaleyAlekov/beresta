using System;

namespace Beresta.SelfTest
{
    /// <summary>Минимальные проверки без тестового фреймворка.
    /// Minimal checks without a test framework.</summary>
    internal static class Expect
    {
        internal static void Equal(object? expected, object? actual, string what)
        {
            if (!Equals(expected, actual))
                throw new InvalidOperationException(what + ": ожидалось [" + expected + "], получено [" + actual + "]");
        }

        internal static void True(bool condition, string what)
        {
            if (!condition)
                throw new InvalidOperationException(what);
        }

        /// <summary>Вызывается после действия, которое обязано было бросить исключение.
        /// Called after an action that was supposed to throw an exception.</summary>
        internal static InvalidOperationException MissingError(string what)
        {
            return new InvalidOperationException(what + ": исключение не было брошено");
        }
    }
}
