using Beresta.Native;

namespace Beresta
{
    /// <summary>Тип, имя, текст и сериализация узла.</summary>
    public partial class XmlNode
    {
        /// <summary>Тип узла.</summary>
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

        /// <summary>Имя по соглашению DOM: тег, цель PI, "#text", "#comment"...</summary>
        public string Name
        {
            get { return NativeCalls.TakeRequiredString(NativeMethods.beresta_node_name(Handle)); }
        }

        /// <summary>Текстовое содержимое; у элемента запись заменяет всех детей одним текстом.</summary>
        public string Text
        {
            get { return NativeCalls.TakeRequiredString(NativeMethods.beresta_node_text(Handle)); }
            set { NativeCalls.Check(NativeMethods.beresta_node_set_text(Handle, value)); }
        }

        /// <summary>Сериализация с потомками; по умолчанию без единого лишнего пробела.</summary>
        public string ToXml(WriteFlags flags = WriteFlags.Default)
        {
            return NativeCalls.TakeRequiredString(NativeMethods.beresta_node_to_string(Handle, (uint)flags));
        }
    }
}
