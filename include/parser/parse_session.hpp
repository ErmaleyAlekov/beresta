#ifndef PARSE_SESSION_HPP
#define PARSE_SESSION_HPP

#include "core/attribute.hpp"
#include "dtd/dtd_model.hpp"
#include "dtd/dtd_reader.hpp"
#include "dtd/dtd_validator.hpp"
#include "parser/internal/expansion_budget.hpp"
#include "parser/internal/input_stack.hpp"
#include "parser/internal/namespace_context.hpp"
#include "parser/parser_options.hpp"
#include "sax/content_handler.hpp"
#include "sax/locator.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::detail
{
    /*
    ** ParseSession — один проход разбора одного текста (уже в UTF-8).
    ** Генерирует SAX-события для ContentHandler; DOM строится одним из
    ** обработчиков (DomBuilder) — так один парсер обслуживает оба режима.
    **
    ** Разбор ИТЕРАТИВНЫЙ: открытые элементы — в явном стеке, текст
    ** сущностей — в стеке источников. Глубокий ввод не переполнит
    ** системный стек вызовов. Цикл run() определяет вид очередной
    ** конструкции и вызывает её обработчик.
    **
    ** Все проверки корректности (вложенность тегов, единственный корень,
    ** пространства имён, сущности) выполняются здесь, поэтому SAX-режим
    ** так же строг, как DOM-режим.
    **
    ** Сессия сама реализует Locator (позиция текущего события) и
    ** ValidityReporter (маршрутизация ошибок валидности в ErrorHandler).
    */
    class ParseSession : public Locator, public dtd::ValidityReporter
    {
    public:
        ParseSession(std::string_view text, const ParserOptions& options, ContentHandler& handler);

        // Полный разбор; синтаксические ошибки — ParseException
        void run();

        SourceLocation getLocation() const override;
        void reportValidity(const std::string& message) override;

    private:
        // Открытый элемент: полное имя и URI его пространства имён
        struct OpenElement
        {
            std::string name;
            std::string namespaceUri;
        };

        // ---- Цикл разбора (parse_session.cpp) ----
        void parseNext();
        bool handleInputEnd();
        void finish();

        // ---- Ошибки (parse_session_errors.cpp) ----
        void runLoop();
        [[noreturn]] void reportFatal(const ParseException& problem) const;
        ParseException toParseException(const DomException& error);
        void markEvent() noexcept;

        // ---- Разметка (parse_markup.cpp) ----
        void parseComment();
        void parseCData();
        void parseProcessingInstruction();
        void parseEndTag();

        // ---- XML-декларация (parse_declaration.cpp) ----
        void parseXmlDeclaration();
        bool readPseudoAttribute(std::string_view name, std::string& value);
        void readStandalone(XmlDeclaration& declaration);

        // ---- DOCTYPE (parse_doctype.cpp) ----
        void parseDoctype();
        void parseInternalSubset(dtd::DtdReader& reader, std::string& subset);
        void loadExternalSubset(const dtd::ExternalId& id);
        void announceDoctype(const std::string& name, const dtd::ExternalId& id,
            const std::string& subset);

        // ---- Открывающие теги (parse_element.cpp) ----
        void parseStartTag();
        void parseAttributes(std::vector<Attribute>& attributes);
        void parseAttribute(std::vector<Attribute>& attributes);

        // ---- События элементов (parse_element_events.cpp) ----
        void emitStartElement(std::string name, std::vector<Attribute>& attributes, bool isEmpty);
        void emitEndElement();
        void buildPlainAttributes(const std::vector<Attribute>& attributes);

        // ---- Пространства имён (parse_namespaces.cpp) ----
        void processNamespaces(OpenElement& element, const std::vector<Attribute>& attributes);
        void resolveAttributeNamespaces(const std::vector<Attribute>& attributes);
        void checkExpandedNames() const;

        // ---- Текст (parse_text.cpp) ----
        void parseText();
        void appendTextChunk(std::string_view chunk);
        void flushText();

        // ---- Ссылки в тексте (parse_text_reference.cpp) ----
        bool handleTextReference();
        void appendTextCharacterReference(std::size_t start);
        void pushGeneralEntity(std::string_view name, std::size_t referenceOffset);
        std::string_view entityText(const dtd::EntityDeclaration& entity);

        InputStack _inputs;
        ParserOptions _options;
        ContentHandler& _handler;
        ExpansionBudget _budget;
        NamespaceContext _namespaces;
        std::shared_ptr<dtd::DtdModel> _dtd;
        std::unique_ptr<dtd::DtdValidator> _validator;
        std::vector<OpenElement> _openElements;
        std::vector<SaxAttribute> _saxAttributes;
        std::string _pendingText;
        bool _pendingSignificant;
        std::size_t _textMainOffset;
        std::size_t _tokenStart;
        std::size_t _eventMainOffset;
        bool _rootSeen;
        bool _doctypeSeen;
    };
}

#endif
