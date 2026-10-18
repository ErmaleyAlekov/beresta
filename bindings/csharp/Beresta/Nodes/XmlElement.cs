using System;
using System.Collections.Generic;
using Beresta.Native;

namespace Beresta
{
    /// <summary>Элемент: узел с именем, атрибутами и пространством имён.
    /// An element: a node with a name, attributes and a namespace.</summary>
    public sealed partial class XmlElement : XmlNode
    {
        internal XmlElement(IntPtr pointer, XmlDocument owner)
            : base(pointer, owner)
        {
        }

        /// <summary>Имя без префикса: у &lt;s:item&gt; — "item".
        /// The name without a prefix: "item" for &lt;s:item&gt;.</summary>
        public string LocalName
        {
            get { return NativeCalls.TakeRequiredString(NativeMethods.beresta_element_local_name(Handle)); }
        }

        /// <summary>URI пространства имён или null, если элемент вне пространств.
        /// The namespace URI or null if the element is in no namespace.</summary>
        public string? NamespaceUri
        {
            get { return NativeCalls.TakeString(NativeMethods.beresta_element_namespace_uri(Handle)); }
        }

        /// <summary>Снимок атрибутов {имя: значение} в порядке документа.
        /// A snapshot of attributes {name: value} in document order.</summary>
        public IReadOnlyList<KeyValuePair<string, string>> Attributes
        {
            get
            {
                long count = NativeMethods.beresta_element_attribute_count(Handle);
                List<KeyValuePair<string, string>> result = new List<KeyValuePair<string, string>>();

                for (long index = 0; index < count; ++index)
                {
                    string name = NativeCalls.TakeRequiredString(NativeMethods.beresta_element_attribute_name(Handle, index));
                    string value = NativeCalls.TakeRequiredString(NativeMethods.beresta_element_attribute_value(Handle, index));
                    result.Add(new KeyValuePair<string, string>(name, value));
                }
                return result;
            }
        }

        /// <inheritdoc />
        public override string ToString()
        {
            return "<" + Name + ">";
        }
    }
}
