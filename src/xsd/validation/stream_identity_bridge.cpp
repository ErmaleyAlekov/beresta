#include "xsd/internal/stream_validator.hpp"
#include "xsd/internal/stream_identity.hpp"
#include "xsd/internal/tree_identity.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Первая область создаёт стратегию ключей: потоковую или запись PSVI
    ** для проверки по дереву. До этого момента ключи ничего не стоят —
    ** как и у обхода дерева, сведения о типах копятся лишь после первой
    ** области.
    **
    ** The first scope creates the key strategy: the streaming one or PSVI recording
    ** for the check over a tree. Until this moment keys cost nothing —
    ** as in the tree traversal, the type information is accumulated only after the first
    ** scope.
    */
    void StreamValidator::openScope(const ElementDecl& declaration)
    {
        if (_identity == nullptr && _settings.identity == IdentityEvaluation::ON_TREE)
            _identity = std::make_unique<TreeIdentityRecorder>(_paths, _log);
        else if (_identity == nullptr)
            _identity = std::make_unique<StreamIdentity>(_paths, _log);
        _identity->openScope(_depth - 1, _frames[_depth - 1].ordinal, _frames[_depth - 1].position, declaration);
    }

    /*
    ** Начало элемента для ключей (стратегия уже создана) — после проверки
    ** атрибутов, чтобы их типы были известны. Если поле ключа выбрало сам элемент, его текст
    ** нужно копить, даже когда тип элемента этого не требует.
    **
    ** An element start for keys (the strategy is already created) — after the attribute
    ** check, so that their types are known. If a key field selected the element itself, its text
    ** must be accumulated even when the element type does not require it.
    */
    void StreamValidator::notifyIdentityStart(const SaxElement& element)
    {
        ElementFrame& frame = _frames[_depth - 1];
        if (frame.attributeTypes.size() != element.attributes.size())
            frame.attributeTypes.assign(element.attributes.size(), nullptr);
        IdentityStart start{_depth - 1, frame.ordinal, element, frame.position, frame.declaration != nullptr,
            frame.attributeTypes};
        if (_identity->elementStarted(start))
            frame.collectText = true;
    }

    // Конец элемента для ключей: тип корректного простого значения и накопленный текст
    // An element end for keys: the type of a valid simple value and the accumulated text
    void StreamValidator::notifyIdentityEnd(const ElementFrame& frame, const SimpleType* valueType)
    {
        std::string_view content;

        if (frame.collectText && !frame.hasChildElement)
            content = _text;
        _identity->elementEnded(IdentityEnd{_depth - 1, frame.ordinal, valueType, content, frame.hasChildElement});
    }

    // ON_TREE: ключи проверяются прежним алгоритмом по готовому дереву
    // ON_TREE: the keys are checked with the former algorithm over the ready tree
    void StreamValidator::checkIdentityOnTree(const Document& document)
    {
        if (_identity != nullptr)
            _identity->checkTree(document);
    }
}
