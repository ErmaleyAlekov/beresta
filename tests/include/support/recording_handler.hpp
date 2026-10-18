#ifndef RECORDING_HANDLER_HPP
#define RECORDING_HANDLER_HPP

#include "beresta.hpp"

#include <string>

namespace support
{
    /*
    ** Обработчик SAX для тестов: записывает каждое событие в строку журнала
    ** вида "start:a[x=1]|text:t|end:a", чтобы порядок событий проверялся
    ** одним сравнением строк.
    **
    ** A SAX handler for tests: records each event into a log string
    ** like "start:a[x=1]|text:t|end:a", so that the event order is checked
    ** with one string comparison.
    */
    class RecordingHandler : public beresta::ContentHandler
    {
    public:
        void startDocument() override;
        void endDocument() override;
        void xmlDeclaration(const beresta::XmlDeclaration& declaration) override;
        void startElement(const beresta::SaxElement& element) override;
        void endElement(const beresta::SaxElement& element) override;
        void characters(std::string_view text) override;
        void cdata(std::string_view text) override;
        void ignorableWhitespace(std::string_view text) override;
        void comment(std::string_view text) override;
        void processingInstruction(std::string_view target, std::string_view data) override;

        const std::string& getLog() const noexcept;
        const std::string& getLastNamespace() const noexcept;
        const std::string& getLastAttributeNamespace() const noexcept;

    private:
        // Добавляет запись в журнал через разделитель '|'
        // Appends a record to the log with the '|' separator
        void record(const std::string& entry);

        std::string _log;
        std::string _lastNamespace;
        std::string _lastAttributeNamespace;
    };
}

#endif
