using System;
using System.Collections.Generic;
using Beresta.Native;

namespace Beresta
{
    /// <summary>Вычисление выражения относительно узла или документа.</summary>
    public sealed partial class XPath
    {
        /// <summary>
        /// Узлы результата в порядке документа. Атрибуты в DOM «Берёсты» —
        /// не узлы и сюда не попадают; их значения возвращает SelectValues.
        /// </summary>
        public IReadOnlyList<XmlNode> SelectNodes(XmlNode context)
        {
            NativeCalls.Check(NativeMethods.beresta_xpath_select(Handle, context.Handle, out IntPtr set));
            List<XmlNode> result = new List<XmlNode>();
            try
            {
                long size = NativeMethods.beresta_nodeset_size(set);
                for (long index = 0; index < size; ++index)
                {
                    XmlNode? node = XmlNode.Wrap(NativeMethods.beresta_nodeset_node(set, index), context.Document);
                    if (node != null)
                        result.Add(node);
                }
                return result;
            }
            finally
            {
                NativeMethods.beresta_nodeset_free(set);
            }
        }

        /// <summary>Строковые значения всех элементов результата, включая атрибуты.</summary>
        public IReadOnlyList<string> SelectValues(XmlNode context)
        {
            NativeCalls.Check(NativeMethods.beresta_xpath_select(Handle, context.Handle, out IntPtr set));
            List<string> result = new List<string>();
            try
            {
                long size = NativeMethods.beresta_nodeset_size(set);
                for (long index = 0; index < size; ++index)
                    result.Add(NativeCalls.TakeRequiredString(NativeMethods.beresta_nodeset_value(set, index)));
                return result;
            }
            finally
            {
                NativeMethods.beresta_nodeset_free(set);
            }
        }

        /// <summary>Результат по правилам функции string().</summary>
        public string EvaluateString(XmlNode context)
        {
            return NativeCalls.TakeRequiredString(NativeMethods.beresta_xpath_string(Handle, context.Handle));
        }

        /// <summary>Результат по правилам функции number(); NaN — не ошибка.</summary>
        public double EvaluateNumber(XmlNode context)
        {
            NativeCalls.Check(NativeMethods.beresta_xpath_number(Handle, context.Handle, out double value));
            return value;
        }

        /// <summary>Результат по правилам функции boolean().</summary>
        public bool EvaluateBoolean(XmlNode context)
        {
            int result = NativeMethods.beresta_xpath_boolean(Handle, context.Handle);

            if (result < 0)
                throw NativeCalls.LastError();
            return result == 1;
        }
    }
}
