using Beresta.Native;

namespace Beresta
{
    /// <summary>Тип, имя, текст и сериализация узла.
    /// Node type, name, text and serialization.</summary>
    public partial class XmlNode
    {
        /// <summary>Тип узла.
        /// The node type.</summary>
        public NodeType Type
        {
            get
            {
                int code = NativeMethods.beresta_node_type(Handle);

                if (code < 0)
                    throw NativeCalls.LastError();
                return (NodeType)code;
            }
        }

        /// <summary>Имя по соглашению DOM: тег, цель PI, "#text", "#comment"...
        /// The name by the DOM convention: a tag, a PI target, "#text", "#comment"...</summary>
        public string Name
        {
            get { return NativeCalls.TakeRequiredString(NativeMethods.beresta_node_name(Handle)); }
        }

        /// <summary>Текстовое содержимое; у элемента запись заменяет всех детей одним текстом.
        /// The text content; for an element, setting it replaces all children with one text node.</summary>
        public string Text
        {
            get { return NativeCalls.TakeRequiredString(NativeMethods.beresta_node_text(Handle)); }
            set { NativeCalls.Check(NativeMethods.beresta_node_set_text(Handle, value)); }
        }

        /// <summary>Сериализация с потомками; по умолчанию без единого лишнего пробела.
        /// Serialization with descendants; by default without a single extra space.</summary>
        public string ToXml(WriteFlags flags = WriteFlags.Default)
        {
            return NativeCalls.TakeRequiredString(NativeMethods.beresta_node_to_string(Handle, (uint)flags));
        }
    }
}
