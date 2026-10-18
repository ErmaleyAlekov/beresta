#ifndef XSD_IDENTITY_TRACKER_HPP
#define XSD_IDENTITY_TRACKER_HPP

#include "core/document.hpp"
#include "core/source_location.hpp"
#include "sax/sax_types.hpp"
#include "xsd/internal/diagnostic_log.hpp"
#include "xsd/schema.hpp"

#include <cstddef>
#include <string_view>
#include <vector>

namespace beresta::xsd::detail
{
    /*
    ** Начало элемента для ограничений идентичности. attributeTypes —
    ** типы атрибутов по их номерам (аналог PSVI): nullptr, если атрибут не
    ** проверялся или его значение ошибочно (тогда ключ сравнивается строкой).
    **
    ** The start of an element for identity constraints. attributeTypes are
    ** the attribute types by their numbers (an analog of the PSVI): nullptr if the attribute was not
    ** validated or its value is invalid (then the key is compared as a string).
    */
    struct IdentityStart
    {
        std::size_t level;
        std::size_t ordinal;
        const SaxElement& element;
        const EventPosition& position;
        bool validated;
        const std::vector<const SimpleType*>& attributeTypes;
    };

    // Конец элемента: тип и текст простого значения (если оно было проверено успешно)
    // The end of an element: the type and the text of the simple value (if it was validated successfully)
    struct IdentityEnd
    {
        std::size_t level;
        std::size_t ordinal;
        const SimpleType* valueType;
        std::string_view text;
        bool hasChildElement;
    };

    /*
    ** IdentityTracker — стратегия проверки ограничений идентичности
    ** (паттерн Strategy). Валидатор создаёт её, лишь встретив первую
    ** область (элемент, объявление которого содержит xs:key / xs:unique /
    ** xs:keyref), — у схем без ключей она ничего не стоит. Две реализации:
    ** потоковая (StreamIdentity) и запись PSVI для проверки по готовому
    ** дереву прежним алгоритмом (TreeIdentityRecorder).
    **
    ** IdentityTracker — the strategy of checking identity constraints
    ** (the Strategy pattern). The validator creates it only on meeting the first
    ** scope (an element whose declaration contains xs:key / xs:unique /
    ** xs:keyref) — for schemas without keys it costs nothing. Two implementations:
    ** the streaming one (StreamIdentity) and PSVI recording for checking over a ready
    ** tree with the former algorithm (TreeIdentityRecorder).
    */
    class IdentityTracker
    {
    public:
        virtual ~IdentityTracker();

        // Проверяемый элемент уровня level — область своих ограничений (до elementStarted)
        // The validated element of the level level is the scope of its constraints (before elementStarted)
        virtual void openScope(std::size_t level, std::size_t ordinal, const EventPosition& position,
            const ElementDecl& declaration) = 0;

        // Начало элемента; true — нужен его текст (поле ключа выбрало этот элемент)
        // The start of an element; true — its text is needed (a key field selected this element)
        virtual bool elementStarted(const IdentityStart& start) = 0;

        // Конец элемента (приходит и для элементов, открытых до создания стратегии)
        // The end of an element (arrives also for elements opened before the strategy was created)
        virtual void elementEnded(const IdentityEnd& end) = 0;

        // Документ дочитан (парсер ещё жив: метки позиций можно перевести); для ON_TREE дальше будет checkTree
        // The document has been read to the end (the parser is still alive: position marks can be converted); for ON_TREE checkTree comes next
        virtual void finishDocument(const Locator* locator) = 0;

        // ON_TREE: проверка по готовому дереву (потоковая стратегия ничего не делает)
        // ON_TREE: the check over a ready tree (the streaming strategy does nothing)
        virtual void checkTree(const Document& document) = 0;
    };
}

#endif
