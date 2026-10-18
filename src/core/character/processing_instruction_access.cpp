#include "core/processing_instruction.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Цель инструкции
    // The instruction target
    const std::string& ProcessingInstruction::getTarget() const noexcept
    {
        return _target;
    }

    // Данные инструкции
    // The instruction data
    const std::string& ProcessingInstruction::getData() const noexcept
    {
        return _data;
    }

    // Сначала проверка, затем присваивание — объект не портится при ошибке
    // First the check, then the assignment — the object is not damaged on error
    void ProcessingInstruction::setData(std::string data)
    {
        validateData(data);
        _data = std::move(data);
    }

    // "?>" внутри данных закрыло бы инструкцию раньше времени
    // "?>" inside the data would close the instruction too early
    void ProcessingInstruction::validateData(std::string_view data)
    {
        if (data.find("?>") != std::string_view::npos)
            throw DomException(DomErrorCode::INVALID_CONTENT,
                i18n::format(i18n::Message::PI_DATA_CONTAINS_END));
    }

    // Сравнение инструкций: тип, цель и данные
    // Comparison of instructions: the type, the target and the data
    bool ProcessingInstruction::isEqualNode(const Node& other) const
    {
        if (other.getType() != NodeType::PROCESSING_INSTRUCTION)
            return false;
        const ProcessingInstruction& instruction = static_cast<const ProcessingInstruction&>(other);
        return instruction._target == _target && instruction._data == _data;
    }
}
