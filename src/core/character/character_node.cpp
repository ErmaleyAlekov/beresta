#include "core/character_node.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Значение изначально пустое. Наследники устанавливают его в теле
    ** своего конструктора через setValue(): к этому моменту объект уже
    ** имеет динамический тип наследника, и вызовется ЕГО validateValue().
    **
    ** The value is initially empty. Subclasses set it in the body of their
    ** constructor through setValue(): by that moment the object already
    ** has the dynamic type of the subclass, and ITS validateValue() is called.
    */
    CharacterNode::CharacterNode() noexcept
        : Node()
    {
    }

    // Возвращает значение без копирования
    // Returns the value without copying
    const std::string& CharacterNode::getValue() const noexcept
    {
        return _value;
    }

    // Сначала проверка (может бросить исключение), затем присваивание
    // First the check (it may throw an exception), then the assignment
    void CharacterNode::setValue(std::string value)
    {
        validateValue(value);
        _value = std::move(value);
    }

    // Текстовое содержимое символьного узла — это его значение
    // The text content of a character node is its value
    std::string CharacterNode::getTextContent() const
    {
        return _value;
    }

    // Базовая реализация ничего не запрещает (подходит для Text)
    // The base implementation forbids nothing (suitable for Text)
    void CharacterNode::validateValue(std::string_view value) const
    {
        (void)value;
    }
}
