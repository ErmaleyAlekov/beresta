#include "parser/parse_session.hpp"
#include "i18n/i18n.hpp"
#include "parser/internal/reference_decoding.hpp"

namespace beresta::detail
{
    /*
    ** Ссылка в тексте. Символьные и пять предопределённых раскрываются
    ** сразу (возвращаем true — продолжаем читать текст). Ссылка на
    ** сущность из DTD кладёт её текст на стек (возвращаем false —
    ** главный цикл продолжит разбор уже из текста сущности).
    **
    ** A reference in text. Character references and the five predefined ones are expanded
    ** at once (true is returned — text reading continues). A reference to an
    ** entity from the DTD pushes its text on the stack (false is returned —
    ** the main loop continues parsing from the entity text).
    */
    bool ParseSession::handleTextReference()
    {
        XmlScanner& scanner = _inputs.current();
        std::size_t start = scanner.getOffset();

        scanner.advance(1);
        if (_openElements.empty())
            scanner.failAt(i18n::format(i18n::Message::REFERENCE_OUTSIDE_ROOT), start);
        if (scanner.peek() == '#')
        {
            appendTextCharacterReference(start);
            return true;
        }
        std::string_view name = scanner.readName();
        if (scanner.peek() != ';')
            scanner.failAt(i18n::format(i18n::Message::REFERENCE_UNTERMINATED), start);
        scanner.advance(1);
        std::optional<char> predefined = predefinedEntity(name);
        if (!predefined.has_value())
        {
            if (_validator)
                _validator->markup();
            pushGeneralEntity(name, start);
            return false;
        }
        _pendingText += predefined.value();
        _pendingSignificant = true;
        return true;
    }

    /*
    ** Проверки ссылки, оплата из бюджета и раскрытие текста сущности.
    ** Необъявленная сущность — фатальная ошибка, только если у документа
    ** нет внешних частей (внешнего подмножества и %pe;) или он standalone;
    ** иначе это ошибка валидности, а ссылка пропускается (раздел 4.1).
    ** При standalone="yes" нельзя ссылаться на внешне объявленную сущность.
    **
    ** Reference checks, payment from the budget and expansion of the entity text.
    ** An undeclared entity is a fatal error only if the document has
    ** no external parts (an external subset and %pe;) or it is standalone;
    ** otherwise it is a validity error, and the reference is skipped (section 4.1).
    ** With standalone="yes" an externally declared entity must not be referenced.
    */
    void ParseSession::pushGeneralEntity(std::string_view name, std::size_t referenceOffset)
    {
        XmlScanner& scanner = _inputs.current();
        const dtd::EntityDeclaration* entity = nullptr;

        if (_dtd)
            entity = _dtd->findEntity(name);
        if (entity == nullptr && _dtd && _dtd->hasExternalParts() && !_standalone)
        {
            if (_validator)
                reportValidity(i18n::format(i18n::Message::ENTITY_UNDECLARED, {name}));
            return;
        }
        if (entity == nullptr)
            scanner.failAt(i18n::format(i18n::Message::ENTITY_UNKNOWN, {name}), referenceOffset);
        if (_standalone && entity->externallyDeclared)
            scanner.failAt(i18n::format(i18n::Message::STANDALONE_EXTERNAL_ENTITY), referenceOffset);
        if (!entity->notation.empty())
            scanner.failAt(i18n::format(i18n::Message::UNPARSED_ENTITY_IN_TEXT, {name}), referenceOffset);
        if (_inputs.isExpanding(entity->name))
            scanner.failAt(i18n::format(i18n::Message::ENTITY_RECURSIVE, {name}), referenceOffset);
        std::string_view text = entityText(*entity);
        _budget.charge(text.size(), _inputs.depth() + 1);
        std::size_t mainOffset = _inputs.mainOffsetFor(referenceOffset);
        _inputs.push(entity->name, text, _openElements.size(), mainOffset);
    }

    // Внутренняя сущность — текст из DTD; внешняя загружается один раз и кэшируется
    // An internal entity is text from the DTD; an external one is loaded once and cached
    std::string_view ParseSession::entityText(const dtd::EntityDeclaration& entity)
    {
        if (!entity.isExternal)
            return entity.value;
        dtd::EntityDeclaration* loaded = _dtd->findEntityForUpdate(entity.name, false);
        if (!loaded->isLoaded)
            dtd::loadExternalEntityText(_options, *loaded);
        return loaded->value;
    }

    // &#N; / &#xH; в тексте: символ добавляется к накопленному тексту
    // &#N; / &#xH; in text: the character is appended to the accumulated text
    void ParseSession::appendTextCharacterReference(std::size_t start)
    {
        XmlScanner& scanner = _inputs.current();
        std::string_view body = scanner.readUntil(";");

        if (!decodeCharacterReference(body.substr(1), _pendingText))
            scanner.failAt(i18n::format(i18n::Message::CHARACTER_REFERENCE_INVALID, {body}), start);
        _pendingSignificant = true;
    }
}
