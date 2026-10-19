#include "xsd/internal/stream_validator.hpp"
#include "xsd/internal/assertion_recorder.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::xsd::detail
{
    // Начало элемента: кадр, затем (если парсер их не разрешил) пространства имён и проверка
    // An element start: the frame, then (if the parser did not resolve them) the namespaces and the check
    void StreamValidator::startElement(const SaxElement& raw)
    {
        openFrame(raw);
        if (_settings.resolveNamespaces)
            checkStart(resolveNames(raw));
        else
            checkStart(raw);
    }

    /*
    ** Приём ребёнка родителем (шаг модели, поиск объявления) или проверка
    ** корня, затем собственная проверка элемента и, наконец, ограничения
    ** идентичности (им нужны уже известные типы атрибутов) и утверждения.
    **
    ** The admission of the child by the parent (a model step, the declaration lookup) or the check
    ** of the root, then the element's own check and, finally, the identity
    ** constraints (they need the already known attribute types) and the assertions.
    */
    void StreamValidator::checkStart(const SaxElement& element)
    {
        const ElementDecl* declaration = nullptr;

        if (_depth == 1)
            declaration = admitRoot(element);
        else
            declaration = admitChild(element);
        if (declaration != nullptr)
            beginValidation(element, *declaration);
        if (_identity != nullptr)
            notifyIdentityStart(element);
        notifyAssertionStart(element);
    }

    /*
    ** Конец элемента: итог простого значения или модели содержимого,
    ** пустота при xsi:nil, затем утверждения, ключи и закрытие уровня.
    ** Накопленный текст принадлежал только этому элементу и больше не нужен.
    **
    ** An element end: the result of the simple value or the content model,
    ** emptiness with xsi:nil, then the assertions, the keys and closing the level.
    ** The accumulated text belonged only to this element and is no longer needed.
    */
    void StreamValidator::endElement()
    {
        ElementFrame& frame = _frames[_depth - 1];
        const SimpleType* valueType = nullptr;

        if (frame.mode == FrameMode::SIMPLE)
            valueType = closeSimple(frame);
        else if (frame.mode == FrameMode::COMPLEX)
            valueType = closeComplex(frame);
        else if (frame.mode == FrameMode::NIL)
            closeNil(frame);
        if (recordingAssertions())
            notifyAssertionEnd(frame, valueType);
        if (_identity != nullptr)
            notifyIdentityEnd(frame, valueType);
        _paths.close();
        _prefixes.pop();
        --_depth;
        _text.clear();
    }

    /*
    ** Текст или CDATA. Как в дереве после DomBuilder: значимый текст —
    ** непробельный или CDATA (если её не сливают с текстом); любой кусок —
    ** уже ребёнок для xsi:nil; значимость нужна лишь составному типу, у
    ** остальных текст не просматривается. Текст копится, только если он нужен
    ** (простое значение или поле ключа) и детей-элементов ещё не было.
    ** Внутри элемента с утверждениями текст ещё и копируется в мини-дерево.
    **
    ** Text or CDATA. As in a tree after DomBuilder: significant text is
    ** non-whitespace or CDATA (unless it is merged with text); any piece is
    ** already a child for xsi:nil; significance is needed only by a complex type, for
    ** the others the text is not scanned. The text is accumulated only if it is needed
    ** (a simple value or a key field) and there have been no child elements yet.
    ** Inside an element with assertions the text is also copied into the mini-tree.
    */
    void StreamValidator::text(std::string_view content, bool isCData)
    {
        if (_depth == 0)
            return;
        ElementFrame& frame = _frames[_depth - 1];
        frame.hasChildNode = true;
        if (!frame.significantText && frame.mode == FrameMode::COMPLEX
            && ((isCData && !_settings.coalesceCData) || !utils::isWhitespaceOnly(content)))
            frame.significantText = true;
        if (frame.collectText && !frame.hasChildElement)
            _text.append(content.data(), content.size());
        if (recordingAssertions())
            _assertions->textAppended(content);
    }

    // Комментарий (если DomBuilder его не выбрасывает) или PI — ребёнок, но не текст
    // A comment (unless DomBuilder discards it) or a PI is a child, but not text
    void StreamValidator::otherChild(bool isComment)
    {
        if (_depth == 0 || (isComment && _settings.discardComments))
            return;
        _frames[_depth - 1].hasChildNode = true;
    }

}
