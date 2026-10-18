#include "support/recording_handler.hpp"

namespace support
{
    // Инструкция обработки
    // A processing instruction
    void RecordingHandler::processingInstruction(std::string_view target, std::string_view data)
    {
        record("pi:" + std::string(target) + "=" + std::string(data));
    }

    // Журнал событий
    // The event log
    const std::string& RecordingHandler::getLog() const noexcept
    {
        return _log;
    }

    // Пространство имён последнего открытого элемента
    // The namespace of the last opened element
    const std::string& RecordingHandler::getLastNamespace() const noexcept
    {
        return _lastNamespace;
    }

    // Пространство имён последнего атрибута с пространством имён
    // The namespace of the last attribute with a namespace
    const std::string& RecordingHandler::getLastAttributeNamespace() const noexcept
    {
        return _lastAttributeNamespace;
    }
}
