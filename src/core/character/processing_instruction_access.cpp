#include "core/processing_instruction.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Цель инструкции
    const std::string& ProcessingInstruction::getTarget() const noexcept
    {
        return _target;
    }

    // Данные инструкции
    const std::string& ProcessingInstruction::getData() const noexcept
    {
        return _data;
    }

    // Сначала проверка, затем присваивание — объект не портится при ошибке
    void ProcessingInstruction::setData(std::string data)
    {
        validateData(data);
        _data = std::move(data);
    }

    // "?>" внутри данных закрыло бы инструкцию раньше времени
    void ProcessingInstruction::validateData(std::string_view data)
    {
        if (data.find("?>") != std::string_view::npos)
            throw DomException(DomErrorCode::INVALID_CONTENT,
                "данные инструкции обработки не могут содержать \"?>\"");
    }

    // Сравнение инструкций: тип, цель и данные
    bool ProcessingInstruction::isEqualNode(const Node& other) const
    {
        if (other.getType() != NodeType::PROCESSING_INSTRUCTION)
            return false;
        const ProcessingInstruction& instruction = static_cast<const ProcessingInstruction&>(other);
        return instruction._target == _target && instruction._data == _data;
    }
}
