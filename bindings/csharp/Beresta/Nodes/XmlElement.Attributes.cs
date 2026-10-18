using Beresta.Native;

namespace Beresta
{
    /// <summary>Чтение и запись атрибутов по имени.
    /// Reading and writing attributes by name.</summary>
    public sealed partial class XmlElement
    {
        /// <summary>Значение атрибута или null, если атрибута нет.
        /// The attribute value or null if there is no such attribute.</summary>
        public string? GetAttribute(string name)
        {
            return NativeCalls.TakeString(NativeMethods.beresta_element_get_attribute(Handle, name));
        }

        /// <summary>Установка или замена значения; недопустимое имя — BerestaException(INVALID_NAME).
        /// Sets or replaces the value; an invalid name throws BerestaException(INVALID_NAME).</summary>
        public void SetAttribute(string name, string value)
        {
            NativeCalls.Check(NativeMethods.beresta_element_set_attribute(Handle, name, value));
        }

        /// <summary>Есть ли атрибут.
        /// Whether the attribute exists.</summary>
        public bool HasAttribute(string name)
        {
            int result = NativeMethods.beresta_element_has_attribute(Handle, name);

            if (result < 0)
                throw NativeCalls.LastError();
            return result == 1;
        }

        /// <summary>true — атрибут был и удалён, false — его не было.
        /// true means the attribute existed and was removed, false means it did not exist.</summary>
        public bool RemoveAttribute(string name)
        {
            int result = NativeMethods.beresta_element_remove_attribute(Handle, name);

            if (result < 0)
                throw NativeCalls.LastError();
            return result == 1;
        }
    }
}
