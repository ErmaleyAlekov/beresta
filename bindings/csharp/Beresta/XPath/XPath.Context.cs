using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Контекст выражения: префиксы пространств имён и переменные $name.
    /// Методы возвращают само выражение — их удобно вызывать цепочкой.
    /// The expression context: namespace prefixes and $name variables.
    /// The methods return the expression itself — convenient for chaining.
    /// </summary>
    public sealed partial class XPath
    {
        /// <summary>Префикс для имён вида p:name в выражении.
        /// A prefix for p:name names in the expression.</summary>
        public XPath Bind(string prefix, string uri)
        {
            NativeCalls.Check(NativeMethods.beresta_xpath_bind_namespace(Handle, prefix, uri));
            return this;
        }

        /// <summary>Строковая переменная.
        /// A string variable.</summary>
        public XPath Set(string name, string value)
        {
            NativeCalls.Check(NativeMethods.beresta_xpath_set_string(Handle, name, value));
            return this;
        }

        /// <summary>Числовая переменная.
        /// A numeric variable.</summary>
        public XPath Set(string name, double value)
        {
            NativeCalls.Check(NativeMethods.beresta_xpath_set_number(Handle, name, value));
            return this;
        }
    }
}
