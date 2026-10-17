#include "parser/parse_session.hpp"
#include "parser/internal/reference_decoding.hpp"

namespace beresta::detail
{
    /*
    ** Ссылка в тексте. Символьные и пять предопределённых раскрываются
    ** сразу (возвращаем true — продолжаем читать текст). Ссылка на
    ** сущность из DTD кладёт её текст на стек (возвращаем false —
    ** главный цикл продолжит разбор уже из текста сущности).
    */
    bool ParseSession::handleTextReference()
    {
        XmlScanner& scanner = _inputs.current();
        std::size_t start = scanner.getOffset();

        scanner.advance(1);
        if (_openElements.empty())
            scanner.failAt("ссылка на сущность вне корневого элемента недопустима", start);
        if (scanner.peek() == '#')
        {
            appendTextCharacterReference(start);
            return true;
        }
        std::string_view name = scanner.readName();
        if (scanner.peek() != ';')
            scanner.failAt("ссылка на сущность не завершена символом ';'", start);
        scanner.advance(1);
        std::optional<char> predefined = predefinedEntity(name);
        if (!predefined.has_value())
        {
            pushGeneralEntity(name, start);
            return false;
        }
        _pendingText += predefined.value();
        _pendingSignificant = true;
        return true;
    }

    // Проверки ссылки, оплата из бюджета и раскрытие текста сущности
    void ParseSession::pushGeneralEntity(std::string_view name, std::size_t referenceOffset)
    {
        XmlScanner& scanner = _inputs.current();
        const dtd::EntityDeclaration* entity = nullptr;

        if (_dtd)
            entity = _dtd->findEntity(name);
        if (entity == nullptr)
            scanner.failAt("неизвестная сущность &" + std::string(name) + ";", referenceOffset);
        if (!entity->notation.empty())
            scanner.failAt("неразбираемая сущность &" + std::string(name)
                + "; не может использоваться в тексте", referenceOffset);
        if (_inputs.isExpanding(entity->name))
            scanner.failAt("рекурсивная сущность &" + std::string(name) + ";", referenceOffset);
        std::string_view text = entityText(*entity);
        _budget.charge(text.size(), _inputs.depth() + 1);
        std::size_t mainOffset = _inputs.mainOffsetFor(referenceOffset);
        _inputs.push(entity->name, text, _openElements.size(), mainOffset);
    }

    // Внутренняя сущность — текст из DTD; внешняя загружается один раз и кэшируется
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
    void ParseSession::appendTextCharacterReference(std::size_t start)
    {
        XmlScanner& scanner = _inputs.current();
        std::string_view body = scanner.readUntil(";");

        if (!decodeCharacterReference(body.substr(1), _pendingText))
            scanner.failAt("недопустимая символьная ссылка &" + std::string(body) + ";", start);
        _pendingSignificant = true;
    }
}
