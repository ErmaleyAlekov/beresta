#include "dtd/dtd_reader.hpp"
#include "i18n/i18n.hpp"
#include "encoding/encoding_detector.hpp"
#include "exception/dom_exception.hpp"
#include "parser/internal/declaration_reader.hpp"
#include "parser/resource_resolver.hpp"

namespace beresta::dtd
{
    // Исчерпанные сущности снимаются, пока на вершине есть непрочитанный текст
    // Exhausted entities are popped while the top has unread text
    void DtdReader::popFinished()
    {
        while (!_frames.empty() && _frames.back().isAtEnd())
        {
            _frames.pop_back();
            _frameInfo.pop_back();
        }
    }

    /*
    ** %name; — ищем объявление, проверяем циклы и бюджет, кладём текст
    ** на стек. Ошибки внутри текста сущности будут сообщаться в позиции
    ** самой ссылки, с пояснением «в параметрической сущности %name;».
    ** Кадр внешней PE получает её собственный URI как базу и признак
    ** «внешний текст»: в нём разрешено всё, что во внешнем подмножестве.
    **
    ** %name; — look up the declaration, check cycles and the budget, push the text
    ** on the stack. Errors inside the entity text will be reported at the position
    ** of the reference itself, with the note "in the parameter entity %name;".
    ** The frame of an external PE gets its own URI as the base and the
    ** "external text" flag: everything allowed in the external subset is allowed in it.
    */
    void DtdReader::expandReference()
    {
        detail::XmlScanner& scanner = current();
        std::size_t start = scanner.getOffset();

        scanner.advance(1);
        std::string name(scanner.readName());
        if (scanner.peek() != ';')
            scanner.failAt(i18n::format(i18n::Message::PE_REFERENCE_UNTERMINATED, {name}), start);
        scanner.advance(1);
        for (const FrameInfo& active : _frameInfo)
        {
            if (active.name == name)
                scanner.failAt(i18n::format(i18n::Message::PE_RECURSIVE, {name}), start);
        }
        ParsePosition anchor = scanner.computePosition(start);
        std::string_view text = parameterEntityText(_context, name);
        const EntityDeclaration* entity = _context.model.findParameterEntity(name);
        FrameInfo info{name, _nextFrameId++, currentBaseUri(), entity->isExternal};

        if (entity->isExternal)
            info.baseUri = entity->resolvedUri;
        _context.budget.charge(text.size(), _frames.size() + 1);
        _frames.emplace_back(text);
        _frames.back().setErrorAnchor(anchor, i18n::format(i18n::Message::IN_PARAMETER_ENTITY, {name, ""}));
        _frameInfo.push_back(info);
    }

    // База текущего текста: у кадра PE — своя, у базового текста — из контекста
    // The base of the current text: a PE frame has its own, the base text takes it from the context
    const std::string& DtdReader::currentBaseUri() const noexcept
    {
        if (_frameInfo.empty())
            return _context.baseUri;
        return _frameInfo.back().baseUri;
    }

    /*
    ** Загрузка внешней сущности через резолвер с перекодированием в UTF-8.
    ** Относительный systemId разрешается от места ОБЪЯВЛЕНИЯ сущности
    ** (entity.baseUri), а не от документа (раздел 4.2.2 XML 1.0).
    **
    ** Loads an external entity through the resolver, transcoding it to UTF-8.
    ** A relative systemId is resolved against the place of the entity DECLARATION
    ** (entity.baseUri), not against the document (section 4.2.2 of XML 1.0).
    */
    void loadExternalEntityText(const ParserOptions& options, EntityDeclaration& entity)
    {
        ResourceResolver* resolver = options.resolver;
        std::string_view base = options.baseUri;

        if (resolver == nullptr)
            throw DomException(DomErrorCode::RESOURCE_ERROR, i18n::format(i18n::Message::EXTERNAL_ENTITY_NO_RESOLVER, {entity.name}));
        if (!entity.baseUri.empty())
            base = entity.baseUri;
        ResourceRequest request{ResourceKind::EXTERNAL_ENTITY, entity.publicId, entity.systemId, base};
        std::optional<Resource> resource = resolver->resolve(request);
        if (!resource.has_value())
            throw DomException(DomErrorCode::RESOURCE_ERROR,
                i18n::format(i18n::Message::EXTERNAL_ENTITY_LOAD, {entity.systemId}));
        encoding::DecodedDocument decoded = encoding::decodeDocument(resource->content, "");
        entity.value = std::string(detail::stripTextDeclaration(decoded.getText()));
        entity.resolvedUri = resource->uri;
        entity.isLoaded = true;
    }

    /*
    ** Текст параметрической сущности; внешняя загружается один раз и
    ** кэшируется в модели. Любая ссылка %pe; делает документ «с внешними
    ** частями»: необъявленные общие сущности тогда — ошибка валидности.
    **
    ** The text of a parameter entity; an external one is loaded once and
    ** cached in the model. Any %pe; reference makes the document one "with external
    ** parts": undeclared general entities are then a validity error.
    */
    std::string_view parameterEntityText(DtdContext& context, std::string_view name)
    {
        EntityDeclaration* entity = context.model.findEntityForUpdate(name, true);

        context.model.markExternalParts();
        if (entity == nullptr)
            throw DomException(DomErrorCode::PARSE_ERROR,
                i18n::format(i18n::Message::PE_UNKNOWN, {name}));
        if (entity->isExternal && !entity->isLoaded)
            loadExternalEntityText(context.options, *entity);
        return entity->value;
    }
}
