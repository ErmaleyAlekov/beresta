#ifndef XSD_ASSERTION_RECORDER_HPP
#define XSD_ASSERTION_RECORDER_HPP

#include "core/element.hpp"
#include "sax/sax_types.hpp"
#include "xsd/internal/assertion_values.hpp"
#include "xsd/internal/prefix_bindings.hpp"
#include "xsd/schema.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace beresta::xsd::detail
{
    /*
    ** Тип узла мини-дерева (аналог PSVI): простой тип корректного значения
    ** или nullptr (нетипизированный узел); elementOnly — составной тип с
    ** содержимым только из элементов: типизированного значения у него нет.
    **
    ** The type of a mini-tree node (an analog of the PSVI): the simple type of a valid value
    ** or nullptr (an untyped node); elementOnly — a complex type with
    ** element-only content: it has no typed value.
    */
    struct NodeTyping
    {
        const SimpleType* type = nullptr;
        bool elementOnly = false;
    };

    /*
    ** AssertionTypes — типизированные значения узлов мини-дерева для XPath
    ** (паттерн Strategy, см. xpath::detail::TypedValueSource). Ключ —
    ** адрес элемента или атрибута: узлы дерева не перемещаются в памяти.
    **
    ** AssertionTypes — the typed values of the mini-tree nodes for XPath
    ** (the Strategy pattern, see xpath::detail::TypedValueSource). The key is
    ** the address of an element or an attribute: tree nodes do not move in memory.
    */
    class AssertionTypes : public xpath::detail::TypedValueSource
    {
    public:
        void annotate(const void* node, NodeTyping typing);

        // Перенос типов вложенного мини-дерева, вставленного в это (assertion_types.cpp)
        // Moving over the types of a nested mini-tree inserted into this one (assertion_types.cpp)
        void absorb(AssertionTypes& other);

        xpath::XPathValue atomize(const xpath::XPathItem& item) const override;
        bool castDate(xpath::ValueType type, std::string_view text, xpath::XPathValue& result) const override;

    private:
        std::unordered_map<const void*, NodeTyping> _typings;
    };

    /*
    ** Мини-дерево одного элемента с утверждениями: копия его поддерева
    ** (элементы, атрибуты, текст) и типы узлов. current — открытый сейчас
    ** элемент, в который идут дети и текст.
    **
    ** The mini-tree of one element with assertions: a copy of its subtree
    ** (elements, attributes, text) and the node types. current is the currently open
    ** element that receives the children and the text.
    */
    struct AssertionCapture
    {
        std::size_t level;
        const ComplexType* type;
        std::unique_ptr<Element> root;
        Element* current;
        AssertionTypes types;
    };

    // Конец элемента внутри мини-дерева: уровень, тип, типы атрибутов и текст по умолчанию
    // An element end inside a mini-tree: the level, the type, the attribute types and the default text
    struct CaptureEnd
    {
        std::size_t level;
        NodeTyping typing;
        const std::vector<const SimpleType*>& attributeTypes;
        std::string_view defaultText;
    };

    /*
    ** AssertionRecorder — буфер для xs:assert при любой проверке (SAX, DOM,
    ** готовое дерево — все идут через поток событий). Утверждению нужен
    ** весь элемент, поэтому поддерево копируется — но ТОЛЬКО у элементов,
    ** тип которых имеет утверждения; остальная проверка по-прежнему
    ** занимает O(глубины). В XSD 1.1 дерево утверждения начинается с самого
    ** элемента (родителя и соседей у него нет) — мини-дерево ровно таково.
    ** Вложенный элемент с утверждениями строит своё мини-дерево, а после
    ** проверки оно без копирования вставляется в дерево внешнего. Стек
    ** мини-деревьев — явный, без рекурсии.
    **
    ** AssertionRecorder — the buffer for xs:assert in any validation (SAX, DOM,
    ** a ready tree — all of them go through the event stream). An assertion needs
    ** the whole element, so the subtree is copied — but ONLY for elements
    ** whose type has assertions; the rest of the validation still
    ** takes O(depth). In XSD 1.1 the assertion tree starts with the element
    ** itself (it has no parent or siblings) — the mini-tree is exactly that.
    ** A nested element with assertions builds its own mini-tree, and after
    ** the check it is inserted into the outer tree without copying. The stack
    ** of mini-trees is explicit, without recursion.
    */
    class AssertionRecorder
    {
    public:
        // Идёт ли запись (открыт хотя бы один элемент с утверждениями)
        // Whether recording is on (at least one element with assertions is open)
        bool recording() const noexcept;

        // Начало элемента уровня level; asserted != nullptr — элемент с утверждениями своего типа
        // The start of an element of the level level; asserted != nullptr — an element with assertions of its type
        void elementStarted(const SaxElement& element, std::size_t level, const ComplexType* asserted,
            const PrefixBindings& prefixes);

        // Текст открытого элемента (копится и становится одним узлом Text)
        // Text of the open element (accumulates and becomes one Text node)
        void textAppended(std::string_view text);

        // Конец элемента; у корня мини-дерева — проверка утверждений, нарушения — в violations
        // An element end; at the root of a mini-tree — the assertion check, violations go to violations
        void elementEnded(const CaptureEnd& end, std::vector<std::string>& violations);

    private:
        // ---- Текст (assertion_recorder.cpp) ----
        // ---- Text (assertion_recorder.cpp) ----
        void flushText();

        // ---- Мини-деревья (assertion_capture.cpp) ----
        // ---- Mini-trees (assertion_capture.cpp) ----
        void startCapture(std::unique_ptr<Element> root, std::size_t level, const ComplexType& type,
            const PrefixBindings& prefixes);
        void annotate(AssertionCapture& capture, Element& element, const CaptureEnd& end);
        void closeCapture(std::vector<std::string>& violations);

        std::vector<AssertionCapture> _captures;
        std::string _pendingText;
    };

    // Копия начала элемента SAX как автономного элемента DOM (имя и все атрибуты, включая xmlns)
    // A copy of a SAX element start as a standalone DOM element (the name and all attributes, including xmlns)
    std::unique_ptr<Element> copyElement(const SaxElement& element);
}

#endif
