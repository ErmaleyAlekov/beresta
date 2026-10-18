#include "core/text.hpp"
#include "utils/xml_escape.hpp"

#include <utility>

namespace beresta
{
    // Текст допускает любое значение, поэтому сразу сохраняем его
    // Text allows any value, so it is stored at once
    Text::Text(std::string value)
        : CharacterNode()
    {
        setValue(std::move(value));
    }

    // Тип узла — текст
    // The node type is text
    NodeType Text::getType() const noexcept
    {
        return NodeType::TEXT;
    }

    // Копия через конструктор копирования — автономна (без родителя)
    // A copy through the copy constructor — standalone (without a parent)
    std::unique_ptr<Node> Text::clone() const
    {
        return std::make_unique<Text>(*this);
    }

    // Текст выводится строго как есть, лишь с экранированием спецсимволов
    // Text is written strictly as is, only with special characters escaped
    void Text::serialize(std::string& output) const
    {
        utils::appendEscapedText(output, getValue());
    }
}
