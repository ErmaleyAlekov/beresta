#include "core/text.hpp"
#include "utils/xml_escape.hpp"

#include <utility>

namespace beresta
{
    // Текст допускает любое значение, поэтому сразу сохраняем его
    Text::Text(std::string value)
        : CharacterNode()
    {
        setValue(std::move(value));
    }

    // Тип узла — текст
    NodeType Text::getType() const noexcept
    {
        return NodeType::TEXT;
    }

    // Копия через конструктор копирования — автономна (без родителя)
    std::unique_ptr<Node> Text::clone() const
    {
        return std::make_unique<Text>(*this);
    }

    // Текст выводится строго как есть, лишь с экранированием спецсимволов
    void Text::serialize(std::string& output) const
    {
        utils::appendEscapedText(output, getValue());
    }
}
