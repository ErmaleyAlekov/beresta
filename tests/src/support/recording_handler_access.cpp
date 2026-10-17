#include "support/recording_handler.hpp"

namespace support
{
    // Инструкция обработки
    void RecordingHandler::processingInstruction(std::string_view target, std::string_view data)
    {
        record("pi:" + std::string(target) + "=" + std::string(data));
    }

    // Журнал событий
    const std::string& RecordingHandler::getLog() const noexcept
    {
        return _log;
    }

    // Пространство имён последнего открытого элемента
    const std::string& RecordingHandler::getLastNamespace() const noexcept
    {
        return _lastNamespace;
    }

    // Пространство имён последнего атрибута с пространством имён
    const std::string& RecordingHandler::getLastAttributeNamespace() const noexcept
    {
        return _lastAttributeNamespace;
    }
}
