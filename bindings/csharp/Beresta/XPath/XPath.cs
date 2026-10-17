using System;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Скомпилированное выражение XPath 1.0: компилируется один раз,
    /// вычисляется сколько угодно раз для разных узлов и документов.
    /// </summary>
    /// <example>
    /// <code>
    /// using XPath query = new XPath("//s:item[@code = $code]").Bind("s", "urn:shop").Set("code", "B2");
    /// string name = query.EvaluateString(document);
    /// </code>
    /// </example>
    public sealed partial class XPath : IDisposable
    {
        private readonly XPathHandle native;

        /// <summary>Компиляция; синтаксическая ошибка — BerestaException(XPATH_ERROR).</summary>
        public XPath(string expression)
        {
            NativeCalls.Check(NativeMethods.beresta_xpath_compile(expression, out IntPtr pointer));
            native = new XPathHandle(pointer);
        }

        /// <summary>Указатель выражения с проверкой, что оно не освобождено.</summary>
        private IntPtr Handle
        {
            get
            {
                if (native.IsClosed)
                    throw new ObjectDisposedException(nameof(XPath));
                return native.DangerousGetHandle();
            }
        }

        /// <summary>Освобождение нативной памяти выражения.</summary>
        public void Dispose()
        {
            native.Dispose();
        }
    }
}
