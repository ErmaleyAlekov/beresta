#ifndef CONTENT_HANDLER_HPP
#define CONTENT_HANDLER_HPP

#include "core/document_type.hpp"
#include "core/xml_declaration.hpp"
#include "sax/locator.hpp"
#include "sax/sax_types.hpp"

#include <string_view>

namespace beresta
{
    /*
    ** ContentHandler — получатель событий потокового разбора (SAX2-стиль).
    **
    ** Все методы имеют пустую реализацию по умолчанию: наследник
    ** переопределяет только нужные события. Так обрабатываются файлы
    ** любого размера при постоянном расходе памяти — дерево не строится.
    **
    ** Порядок: setDocumentLocator, startDocument, [xmlDeclaration],
    ** [documentType], события содержимого, endDocument.
    ** Текст может прийти несколькими вызовами characters() подряд.
    */
    class ContentHandler
    {
    public:
        virtual ~ContentHandler();

        // Источник позиции; действителен до конца разбора
        virtual void setDocumentLocator(const Locator& locator);

        virtual void startDocument();
        virtual void endDocument();

        // XML-декларация в начале документа
        virtual void xmlDeclaration(const XmlDeclaration& declaration);

        // Объявление DOCTYPE вместе с разобранной моделью DTD
        virtual void documentType(const DocumentType& documentType);

        virtual void startElement(const SaxElement& element);
        virtual void endElement(const SaxElement& element);

        // Значимый текст (сущности уже раскрыты)
        virtual void characters(std::string_view text);

        // Содержимое секции CDATA
        virtual void cdata(std::string_view text);

        // Текст из одних пробелов между тегами (отбрасываемое форматирование)
        virtual void ignorableWhitespace(std::string_view text);

        virtual void comment(std::string_view text);
        virtual void processingInstruction(std::string_view target, std::string_view data);
    };
}

#endif
