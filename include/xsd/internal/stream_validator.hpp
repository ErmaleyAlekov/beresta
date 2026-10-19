#ifndef XSD_STREAM_VALIDATOR_HPP
#define XSD_STREAM_VALIDATOR_HPP

#include "content/content_cursor.hpp"
#include "core/source_location.hpp"
#include "parser/error_handler.hpp"
#include "sax/locator.hpp"
#include "sax/sax_types.hpp"
#include "utils/seeded_hash.hpp"
#include "xsd/internal/diagnostic_log.hpp"
#include "xsd/internal/id_registry.hpp"
#include "xsd/internal/identity_tracker.hpp"
#include "xsd/internal/path_tracker.hpp"
#include "xsd/internal/prefix_bindings.hpp"
#include "xsd/schema.hpp"
#include "xsd/validating_handler.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace beresta::xsd::detail
{
    // Как проверяется открытый элемент
    // How an open element is validated
    enum class FrameMode
    {
        UNCHECKED,  // не проверяется (нет объявления, xs:any skip, родитель не составной)
                    // not validated (no declaration, xs:any skip, the parent is not complex)
        SIMPLE,     // простой тип: ни детей, ни атрибутов, текст по типу
                    // a simple type: neither children nor attributes, the text by the type
        COMPLEX,    // составной тип: атрибуты, модель содержимого, текст
                    // a complex type: attributes, the content model, the text
        NIL,        // xsi:nil="true": содержимое обязано быть пустым
                    // xsi:nil="true": the content must be empty
        OPAQUE      // проверен сам элемент, но не содержимое (xs:anyType, ошибка xsi:type)
                    // the element itself is validated, but not the content (xs:anyType, an xsi:type error)
    };

    // Значения служебных атрибутов xsi:type и xsi:nil (найдены одним проходом по атрибутам)
    // The values of the service attributes xsi:type and xsi:nil (found in one pass over the attributes)
    struct XsiValues
    {
        std::string_view type;
        std::string_view nil;
        bool hasType = false;
        bool hasNil = false;
    };

    // Ребёнок элемента с отложенной проверкой модели: где сообщить, если он окажется лишним
    // A child of an element with a deferred model check: where to report if it turns out to be extra
    struct BufferedChild
    {
        std::size_t step;
        EventPosition position;
    };

    /*
    ** Кадр открытого элемента. Кадры лежат в массиве по глубине и
    ** переиспользуются: память курсора модели, списков и строк остаётся
    ** от прошлых элементов той же глубины, поэтому проверка после прогрева
    ** почти не выделяет память.
    **
    ** The frame of an open element. Frames lie in an array by depth and
    ** are reused: the memory of the model cursor, the lists and the strings stays
    ** from the previous elements of the same depth, so validation after warm-up
    ** allocates almost no memory.
    */
    struct ElementFrame
    {
        std::size_t ordinal = 0;
        EventPosition position;
        FrameMode mode = FrameMode::UNCHECKED;
        const ElementDecl* declaration = nullptr;
        const ComplexType* complex = nullptr;
        const SimpleType* valueType = nullptr;
        bool hasChildElement = false;
        bool hasChildNode = false;
        bool significantText = false;
        bool collectText = false;
        bool modelFailed = false;
        bool reportedChildren = false;
        bool declaresNamespaces = false;
        std::size_t childCount = 0;
        content::CursorPlan plan = content::CursorPlan::GREEDY_SEQUENCE;
        content::ContentCursor cursor;
        std::vector<std::string> pendingAttributes;
        std::size_t pendingCount = 0;
        std::vector<BufferedChild> bufferedChildren;
        std::size_t bufferedCount = 0;
        std::vector<const SimpleType*> attributeTypes;
    };

    /*
    ** StreamValidator — проверка документа по схеме по событиям SAX, без
    ** дерева. Каждое событие обрабатывается сразу: начало элемента — тип,
    ** xsi:type, xsi:nil, атрибуты и шаг модели содержимого у родителя;
    ** текст — накопление для простого значения; конец элемента — значение
    ** и завершение модели. Ошибки не прерывают проверку (полный список), а
    ** их порядок, пути и позиции совпадают с прежним обходом дерева.
    **
    ** StreamValidator — validating a document against a schema by SAX events, without
    ** a tree. Every event is processed at once: an element start — the type,
    ** xsi:type, xsi:nil, the attributes and a content model step at the parent;
    ** text — accumulation for a simple value; an element end — the value
    ** and finishing the model. Errors do not stop the validation (the full list), and
    ** their order, paths and positions are the same as in the former tree traversal.
    */
    class StreamValidator
    {
    public:
        StreamValidator(const Schema& schema, const ValidationSettings& settings);
        ~StreamValidator();

        StreamValidator(const StreamValidator&) = delete;
        StreamValidator& operator=(const StreamValidator&) = delete;

        // Источник позиций событий (может отсутствовать — тогда позиции нулевые)
        // The source of event positions (may be absent — then the positions are zero)
        void setLocator(const Locator* locator) noexcept;

        // ---- События (stream_events.cpp) ----
        // ---- Events (stream_events.cpp) ----
        void startElement(const SaxElement& element);
        void endElement();
        void text(std::string_view content, bool isCData);
        void otherChild(bool isComment);
        void checkIdentityOnTree(const Document& document);

        // Конец документа: висячие IDREF и завершение ключей
        // The end of the document: dangling IDREFs and finishing the keys
        void endDocument();

        // Итоговый список ошибок с собранными путями
        // The final error list with assembled paths
        std::vector<Diagnostic> takeDiagnostics();

    private:
        // ---- Открытие элемента (stream_open.cpp, stream_events.cpp) ----
        // ---- Opening an element (stream_open.cpp) ----
        void openFrame(const SaxElement& element);
        void checkStart(const SaxElement& element);
        const ElementDecl* admitRoot(const SaxElement& element);
        const ElementDecl* admitChild(const SaxElement& element);
        void stepModel(ElementFrame& parent, const SaxElement& element);
        const ElementDecl* childDeclaration(const ComplexType& type, const SaxElement& element);

        // ---- Начало проверки элемента (stream_begin.cpp) ----
        // ---- The start of an element check (stream_begin.cpp) ----
        void beginValidation(const SaxElement& element, const ElementDecl& declaration);
        bool applyXsiType(const XsiValues& xsi, const ElementDecl& declaration, TypeRef& type);
        bool isNil(const XsiValues& xsi, const ElementDecl& declaration);
        void beginSimple(const SaxElement& element, const SimpleType& type);
        void beginComplex(const SaxElement& element, const ComplexType& type);

        // ---- Атрибуты (stream_attributes.cpp) ----
        // ---- Attributes (stream_attributes.cpp) ----
        void validateAttributes(const SaxElement& element, const ComplexType& type);
        void checkAttribute(const AttributeUse& use, const SaxAttribute& attribute, std::size_t index);
        void checkRequiredAttributes(const SaxElement& element, const ComplexType& type, std::uint64_t seen);

        // ---- Закрытие элемента (stream_close.cpp) ----
        // ---- Closing an element (stream_close.cpp) ----
        const SimpleType* closeSimple(ElementFrame& frame);
        const SimpleType* closeComplex(ElementFrame& frame);
        void finishModel(ElementFrame& frame);
        content::CursorPlan planFor(const content::Particle& particle);

        // ---- Значения и ID (stream_values.cpp) ----
        // ---- Values and IDs (stream_values.cpp) ----
        const SimpleType* checkValue(const SimpleType& type, std::string_view text, CheckPhase phase);
        void recordIdentity(const SimpleType& type, std::string_view value, CheckPhase phase);
        void finishReferences();

        // ---- Сообщения и ограничения идентичности (stream_report.cpp, stream_identity_bridge.cpp) ----
        // ---- Messages and identity constraints (stream_report.cpp, stream_identity_bridge.cpp) ----
        void report(std::size_t level, CheckPhase phase, std::string message);
        void reportAtChild(CheckPhase phase, std::string message);
        void openScope(const ElementDecl& declaration);
        void notifyIdentityStart(const SaxElement& element);
        void notifyIdentityEnd(const ElementFrame& frame, const SimpleType* valueType);

        // ---- Пространства имён без поддержки парсера (stream_namespaces.cpp) ----
        // ---- Namespaces without the parser's support (stream_namespaces.cpp) ----
        SaxElement resolveNames(const SaxElement& element);

        const Schema& _schema;
        ValidationSettings _settings;
        const Locator* _locator;
        std::vector<ElementFrame> _frames;
        std::size_t _depth;
        std::size_t _nextOrdinal;
        PathTracker _paths;
        PrefixBindings _prefixes;
        DiagnosticLog _log;
        IdRegistry _ids;
        std::string _normalized;
        std::vector<std::pair<std::string, std::size_t>> _references;
        EventPosition _rootPosition;
        std::string _text;
        std::unordered_map<const content::Particle*, content::CursorPlan> _plans;
        const content::Particle* _lastParticle;
        content::CursorPlan _lastPlan;
        std::unique_ptr<IdentityTracker> _identity;
        std::vector<SaxAttribute> _resolvedAttributes;
        std::vector<std::string> _resolvedUris;
        std::string _resolvedElementUri;
    };
}

#endif
