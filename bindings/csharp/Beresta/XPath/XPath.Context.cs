using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Контекст выражения: префиксы пространств имён и переменные $name.
    /// Методы возвращают само выражение — их удобно вызывать цепочкой.
    /// </summary>
    public sealed partial class XPath
    {
        /// <summary>Префикс для имён вида p:name в выражении.</summary>
        public XPath Bind(string prefix, string uri)
        {
            NativeCalls.Check(NativeMethods.beresta_xpath_bind_namespace(Handle, prefix, uri));
            return this;
        }

        /// <summary>Строковая переменная.</summary>
        public XPath Set(string name, string value)
        {
            NativeCalls.Check(NativeMethods.beresta_xpath_set_string(Handle, name, value));
            return this;
        }

        /// <summary>Числовая переменная.</summary>
        public XPath Set(string name, double value)
        {
            NativeCalls.Check(NativeMethods.beresta_xpath_set_number(Handle, name, value));
            return this;
        }
    }
}
