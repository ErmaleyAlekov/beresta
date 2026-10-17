using Beresta.Native;

namespace Beresta
{
    /// <summary>Чтение и запись атрибутов по имени.</summary>
    public sealed partial class XmlElement
    {
        /// <summary>Значение атрибута или null, если атрибута нет.</summary>
        public string? GetAttribute(string name)
        {
            return NativeCalls.TakeString(NativeMethods.beresta_element_get_attribute(Handle, name));
        }

        /// <summary>Установка или замена значения; недопустимое имя — BerestaException(INVALID_NAME).</summary>
        public void SetAttribute(string name, string value)
        {
            NativeCalls.Check(NativeMethods.beresta_element_set_attribute(Handle, name, value));
        }

        /// <summary>Есть ли атрибут.</summary>
        public bool HasAttribute(string name)
        {
            int result = NativeMethods.beresta_element_has_attribute(Handle, name);

            if (result < 0)
                throw NativeCalls.LastError();
            return result == 1;
        }

        /// <summary>true — атрибут был и удалён, false — его не было.</summary>
        public bool RemoveAttribute(string name)
        {
            int result = NativeMethods.beresta_element_remove_attribute(Handle, name);

            if (result < 0)
                throw NativeCalls.LastError();
            return result == 1;
        }
    }
}
