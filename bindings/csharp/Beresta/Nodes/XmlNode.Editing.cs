using System;
using Beresta.Native;

namespace Beresta
{
    /// <summary>
    /// Добавление новых узлов в конец детей. Каждый метод возвращает созданный
    /// узел: <c>root.AppendElement("item").AppendText("Чай")</c>.
    /// Appends new nodes to the end of the children. Each method returns the created
    /// node: <c>root.AppendElement("item").AppendText("Чай")</c>.
    /// </summary>
    public partial class XmlNode
    {
        /// <summary>Новый элемент; у пустого документа он становится корнем.
        /// A new element; in an empty document it becomes the root.</summary>
        public XmlElement AppendElement(string name)
        {
            return (XmlElement)Required(NativeMethods.beresta_node_append_element(Handle, name));
        }

        /// <summary>Новый текстовый узел.
        /// A new text node.</summary>
        public XmlNode AppendText(string text)
        {
            return Required(NativeMethods.beresta_node_append_text(Handle, text));
        }

        /// <summary>Новый комментарий.
        /// A new comment.</summary>
        public XmlNode AppendComment(string text)
        {
            return Required(NativeMethods.beresta_node_append_comment(Handle, text));
        }

        /// <summary>Новая секция CDATA.
        /// A new CDATA section.</summary>
        public XmlNode AppendCData(string text)
        {
            return Required(NativeMethods.beresta_node_append_cdata(Handle, text));
        }

        /// <summary>Новая инструкция обработки &lt;?target data?&gt;.
        /// A new processing instruction &lt;?target data?&gt;.</summary>
        public XmlNode AppendProcessingInstruction(string target, string? data = null)
        {
            return Required(NativeMethods.beresta_node_append_pi(Handle, target, data));
        }
    }
}
