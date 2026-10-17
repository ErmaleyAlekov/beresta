#include "support/recording_handler.hpp"

namespace support
{
    // Начало документа
    void RecordingHandler::startDocument()
    {
        record("begin");
    }

    // Конец документа
    void RecordingHandler::endDocument()
    {
        record("finish");
    }

    // Версия из декларации
    void RecordingHandler::xmlDeclaration(const beresta::XmlDeclaration& declaration)
    {
        record("decl:" + declaration.version);
    }

    // Элемент с атрибутами; запоминаются пространства имён для отдельных проверок
    void RecordingHandler::startElement(const beresta::SaxElement& element)
    {
        std::string entry = "start:" + std::string(element.name) + "[";

        for (const beresta::SaxAttribute& attribute : element.attributes)
        {
            entry += std::string(attribute.name) + "=" + std::string(attribute.value);
            if (!attribute.namespaceUri.empty())
                _lastAttributeNamespace = std::string(attribute.namespaceUri);
        }
        _lastNamespace = std::string(element.namespaceUri);
        record(entry + "]");
    }

    // Добавление записи через разделитель
    void RecordingHandler::record(const std::string& entry)
    {
        if (!_log.empty())
            _log += '|';
        _log += entry;
    }
}
