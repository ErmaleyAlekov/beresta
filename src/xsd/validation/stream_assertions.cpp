#include "xsd/internal/stream_validator.hpp"
#include "xsd/internal/assertion_recorder.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    // Открыт ли сейчас элемент с утверждениями (тогда события копируются в мини-дерево)
    // Whether an element with assertions is open now (then the events are copied into a mini-tree)
    bool StreamValidator::recordingAssertions() const noexcept
    {
        return _assertions != nullptr && _assertions->recording();
    }

    /*
    ** Начало элемента для утверждений — после проверки типа (xsi:type уже
    ** применён). Элемент составного типа с xs:assert открывает мини-дерево;
    ** внутри мини-дерева копируется каждый элемент. Буфер создаётся лишь
    ** при первом таком элементе — схемы без утверждений ничего не платят.
    **
    ** An element start for assertions — after the type check (xsi:type has already
    ** been applied). An element of a complex type with xs:assert opens a mini-tree;
    ** inside a mini-tree every element is copied. The buffer is created only
    ** at the first such element — schemas without assertions pay nothing.
    */
    void StreamValidator::notifyAssertionStart(const SaxElement& element)
    {
        const ElementFrame& frame = _frames[_depth - 1];
        const ComplexType* asserted = nullptr;

        if (frame.mode == FrameMode::COMPLEX && !frame.complex->assertions.empty())
            asserted = frame.complex;
        if (asserted == nullptr && !recordingAssertions())
            return;
        if (_assertions == nullptr)
            _assertions = std::make_unique<AssertionRecorder>();
        _assertions->elementStarted(element, _depth - 1, asserted, _prefixes);
    }

    // Значение по умолчанию (default, иначе fixed) пустого элемента с корректным простым значением
    // The default value (default, otherwise fixed) of an empty element with a valid simple value
    static std::string_view defaultTextOf(const ElementFrame& frame, const SimpleType* valueType,
        std::string_view text)
    {
        const ElementDecl* declaration = frame.declaration;

        if (valueType == nullptr || !text.empty() || declaration == nullptr)
            return std::string_view();
        if (declaration->defaultValue.has_value())
            return declaration->defaultValue.value();
        if (declaration->fixedValue.has_value())
            return declaration->fixedValue.value();
        return std::string_view();
    }

    /*
    ** Конец элемента внутри мини-дерева: тип элемента (корректное простое
    ** значение, «только элементы» или нетипизированный) и его атрибутов. У
    ** корня мини-дерева вычисляются утверждения; каждое нарушение — ошибка
    ** этого элемента (его путь и позиция начала) на этапе ASSERTION.
    **
    ** An element end inside a mini-tree: the type of the element (a valid simple
    ** value, "element-only" or untyped) and of its attributes. At the
    ** root of a mini-tree the assertions are evaluated; each violation is an error
    ** of this element (its path and start position) at the ASSERTION phase.
    */
    void StreamValidator::notifyAssertionEnd(const ElementFrame& frame, const SimpleType* valueType)
    {
        NodeTyping typing;
        std::vector<std::string> violations;

        typing.type = valueType;
        typing.elementOnly = frame.mode == FrameMode::COMPLEX && frame.complex->content == ContentType::ELEMENT_ONLY;
        _assertions->elementEnded(CaptureEnd{_depth - 1, typing, frame.attributeTypes,
            defaultTextOf(frame, valueType, _text)}, violations);
        for (std::string& message : violations)
            report(_depth - 1, CheckPhase::ASSERTION, std::move(message));
    }
}
