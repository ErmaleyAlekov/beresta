using System;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Скомпилированное выражение XPath 1.0: компилируется один раз,
    /// вычисляется сколько угодно раз для разных узлов и документов.
    /// A compiled XPath 1.0 expression: compiled once,
    /// evaluated any number of times for different nodes and documents.
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

        /// <summary>Компиляция; синтаксическая ошибка — BerestaException(XPATH_ERROR).
        /// Compilation; a syntax error throws BerestaException(XPATH_ERROR).</summary>
        public XPath(string expression)
        {
            NativeCalls.Check(NativeMethods.beresta_xpath_compile(expression, out IntPtr pointer));
            native = new XPathHandle(pointer);
        }

        /// <summary>Указатель выражения с проверкой, что оно не освобождено.
        /// The expression pointer, checking that it is not released.</summary>
        private IntPtr Handle
        {
            get
            {
                if (native.IsClosed)
                    throw new ObjectDisposedException(nameof(XPath), "Берёста / Beresta: " + BerestaLibrary.Message(2));
                return native.DangerousGetHandle();
            }
        }

        /// <summary>Освобождение нативной памяти выражения.
        /// Releases the native memory of the expression.</summary>
        public void Dispose()
        {
            native.Dispose();
        }
    }
}
