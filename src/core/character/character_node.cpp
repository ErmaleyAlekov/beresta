#include "core/character_node.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Значение изначально пустое. Наследники устанавливают его в теле
    ** своего конструктора через setValue(): к этому моменту объект уже
    ** имеет динамический тип наследника, и вызовется ЕГО validateValue().
    */
    CharacterNode::CharacterNode() noexcept
        : Node()
    {
    }

    // Возвращает значение без копирования
    const std::string& CharacterNode::getValue() const noexcept
    {
        return _value;
    }

    // Сначала проверка (может бросить исключение), затем присваивание
    void CharacterNode::setValue(std::string value)
    {
        validateValue(value);
        _value = std::move(value);
    }

    // Текстовое содержимое символьного узла — это его значение
    std::string CharacterNode::getTextContent() const
    {
        return _value;
    }

    // Базовая реализация ничего не запрещает (подходит для Text)
    void CharacterNode::validateValue(std::string_view value) const
    {
        (void)value;
    }
}
