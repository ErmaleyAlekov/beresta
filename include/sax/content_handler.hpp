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
    **
    ** ContentHandler — the receiver of streaming parse events (SAX2 style).
    ** All methods have an empty default implementation: a subclass
    ** overrides only the events it needs. This way files of any size are processed
    ** with constant memory use — no tree is built.
    ** Order: setDocumentLocator, startDocument, [xmlDeclaration],
    ** [documentType], content events, endDocument.
    ** Text may arrive in several characters() calls in a row.
    */
    class ContentHandler
    {
    public:
        virtual ~ContentHandler();

        // Источник позиции; действителен до конца разбора
        // The position source; valid until the end of parsing
        virtual void setDocumentLocator(const Locator& locator);

        virtual void startDocument();
        virtual void endDocument();

        // XML-декларация в начале документа
        // The XML declaration at the start of the document
        virtual void xmlDeclaration(const XmlDeclaration& declaration);

        // Объявление DOCTYPE вместе с разобранной моделью DTD
        // The DOCTYPE declaration together with the parsed DTD model
        virtual void documentType(const DocumentType& documentType);

        virtual void startElement(const SaxElement& element);
        virtual void endElement(const SaxElement& element);

        // Значимый текст (сущности уже раскрыты)
        // Significant text (entities are already expanded)
        virtual void characters(std::string_view text);

        // Содержимое секции CDATA
        // The content of a CDATA section
        virtual void cdata(std::string_view text);

        // Текст из одних пробелов между тегами (отбрасываемое форматирование)
        // Whitespace-only text between tags (discardable formatting)
        virtual void ignorableWhitespace(std::string_view text);

        virtual void comment(std::string_view text);
        virtual void processingInstruction(std::string_view target, std::string_view data);

        /*
        ** Инструкция обработки из DTD (внутреннего или внешнего подмножества).
        ** Раздел 2.6 XML 1.0: PI обязаны передаваться приложению, где бы ни
        ** стояли. По умолчанию вызывает processingInstruction(), поэтому
        ** обычный SAX-обработчик получает их без доработок; приходят они
        ** до события documentType(). DomBuilder их пропускает: у DOM для PI
        ** внутри DTD нет места (их текст остаётся в getInternalSubset()).
        **
        ** A processing instruction from the DTD (the internal or external subset).
        ** Section 2.6 of XML 1.0: PIs must be passed to the application wherever
        ** they are. By default it calls processingInstruction(), so an ordinary
        ** SAX handler receives them without changes; they arrive before the
        ** documentType() event. DomBuilder skips them: the DOM has no place for PIs
        ** inside a DTD (their text stays in getInternalSubset()).
        */
        virtual void dtdProcessingInstruction(std::string_view target, std::string_view data);
    };
}

#endif
