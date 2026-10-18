#include "core/processing_instruction.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/string_utils.hpp"
#include "utils/xml_chars.hpp"

#include <utility>

namespace beresta
{
    // Цель — корректное XML-имя, но не зарезервированное "xml"; данные — без "?>"
    // The target is a valid XML name, but not the reserved "xml"; the data has no "?>"
    ProcessingInstruction::ProcessingInstruction(std::string target, std::string data)
        : Node(), _target(std::move(target))
    {
        if (!utils::isValidName(_target))
            throw DomException(DomErrorCode::INVALID_NAME,
                i18n::format(i18n::Message::PI_TARGET_INVALID, {_target}));
        if (utils::equalsIgnoreCaseAscii(_target, "xml"))
            throw DomException(DomErrorCode::INVALID_NAME,
                i18n::format(i18n::Message::PI_TARGET_RESERVED));
        setData(std::move(data));
    }

    // Тип узла — инструкция обработки
    // The node type is a processing instruction
    NodeType ProcessingInstruction::getType() const noexcept
    {
        return NodeType::PROCESSING_INSTRUCTION;
    }

    // Автономная копия инструкции
    // A standalone copy of the instruction
    std::unique_ptr<Node> ProcessingInstruction::clone() const
    {
        return std::make_unique<ProcessingInstruction>(*this);
    }

    // Формат: <?target?> или <?target data?> — пробел только как разделитель
    // The format: <?target?> or <?target data?> — a space only as a separator
    void ProcessingInstruction::serialize(std::string& output) const
    {
        output += "<?";
        output += _target;
        if (!_data.empty())
        {
            output += ' ';
            output += _data;
        }
        output += "?>";
    }

    // Текстовое содержимое — данные инструкции
    // The text content is the instruction data
    std::string ProcessingInstruction::getTextContent() const
    {
        return _data;
    }
}
