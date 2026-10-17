using System;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Добавление новых узлов в конец детей. Каждый метод возвращает созданный
    /// узел: <c>root.AppendElement("item").AppendText("Чай")</c>.
    /// </summary>
    public partial class XmlNode
    {
        /// <summary>Новый элемент; у пустого документа он становится корнем.</summary>
        public XmlElement AppendElement(string name)
        {
            return (XmlElement)Required(NativeMethods.beresta_node_append_element(Handle, name));
        }

        /// <summary>Новый текстовый узел.</summary>
        public XmlNode AppendText(string text)
        {
            return Required(NativeMethods.beresta_node_append_text(Handle, text));
        }

        /// <summary>Новый комментарий.</summary>
        public XmlNode AppendComment(string text)
        {
            return Required(NativeMethods.beresta_node_append_comment(Handle, text));
        }

        /// <summary>Новая секция CDATA.</summary>
        public XmlNode AppendCData(string text)
        {
            return Required(NativeMethods.beresta_node_append_cdata(Handle, text));
        }

        /// <summary>Новая инструкция обработки &lt;?target data?&gt;.</summary>
        public XmlNode AppendProcessingInstruction(string target, string? data = null)
        {
            return Required(NativeMethods.beresta_node_append_pi(Handle, target, data));
        }
    }
}
