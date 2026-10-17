#include "core/processing_instruction.hpp"
#include "exception/dom_exception.hpp"
#include "utils/string_utils.hpp"
#include "utils/xml_chars.hpp"

#include <utility>

namespace beresta
{
    // Цель — корректное XML-имя, но не зарезервированное "xml"; данные — без "?>"
    ProcessingInstruction::ProcessingInstruction(std::string target, std::string data)
        : Node(), _target(std::move(target))
    {
        if (!utils::isValidName(_target))
            throw DomException(DomErrorCode::INVALID_NAME,
                "недопустимая цель инструкции обработки: \"" + _target + "\"");
        if (utils::equalsIgnoreCaseAscii(_target, "xml"))
            throw DomException(DomErrorCode::INVALID_NAME,
                "цель \"xml\" зарезервирована: используйте Document::setDeclaration()");
        setData(std::move(data));
    }

    // Тип узла — инструкция обработки
    NodeType ProcessingInstruction::getType() const noexcept
    {
        return NodeType::PROCESSING_INSTRUCTION;
    }

    // Автономная копия инструкции
    std::unique_ptr<Node> ProcessingInstruction::clone() const
    {
        return std::make_unique<ProcessingInstruction>(*this);
    }

    // Формат: <?target?> или <?target data?> — пробел только как разделитель
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
    std::string ProcessingInstruction::getTextContent() const
    {
        return _data;
    }
}
