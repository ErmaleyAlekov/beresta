#include "dtd/dtd_reader.hpp"
#include "encoding/encoding_detector.hpp"
#include "exception/dom_exception.hpp"
#include "parser/internal/reference_decoding.hpp"
#include "parser/resource_resolver.hpp"

namespace beresta::dtd
{
    // Исчерпанные сущности снимаются, пока на вершине есть непрочитанный текст
    void DtdReader::popFinished()
    {
        while (!_frames.empty() && _frames.back().isAtEnd())
        {
            _frames.pop_back();
            _frameNames.pop_back();
        }
    }

    /*
    ** %name; — ищем объявление, проверяем циклы и бюджет, кладём текст
    ** на стек. Ошибки внутри текста сущности будут сообщаться в позиции
    ** самой ссылки, с пояснением «в параметрической сущности %name;».
    */
    void DtdReader::expandReference()
    {
        detail::XmlScanner& scanner = current();
        std::size_t start = scanner.getOffset();

        scanner.advance(1);
        std::string name(scanner.readName());
        if (scanner.peek() != ';')
            scanner.failAt("ссылка %" + name + " не завершена символом ';'", start);
        scanner.advance(1);
        for (const std::string& active : _frameNames)
        {
            if (active == name)
                scanner.failAt("рекурсивная параметрическая сущность %" + name + ";", start);
        }
        ParsePosition anchor = scanner.computePosition(start);
        std::string_view text = parameterEntityText(_context, name);
        _context.budget.charge(text.size(), _frames.size() + 1);
        _frames.emplace_back(text);
        _frames.back().setErrorAnchor(anchor, "в параметрической сущности %" + name + ";: ");
        _frameNames.push_back(name);
    }

    // Загрузка внешней сущности через резолвер с перекодированием в UTF-8
    void loadExternalEntityText(const ParserOptions& options, EntityDeclaration& entity)
    {
        ResourceResolver* resolver = options.resolver;

        if (resolver == nullptr)
            throw DomException(DomErrorCode::RESOURCE_ERROR, "внешняя сущность "
                + entity.name + " не загружена: не задан ResourceResolver");
        ResourceRequest request{ResourceKind::EXTERNAL_ENTITY, entity.publicId,
            entity.systemId, options.baseUri};
        std::optional<Resource> resource = resolver->resolve(request);
        if (!resource.has_value())
            throw DomException(DomErrorCode::RESOURCE_ERROR,
                "не удалось загрузить внешнюю сущность \"" + entity.systemId + "\"");
        encoding::DecodedDocument decoded = encoding::decodeDocument(resource->content, "");
        entity.value = std::string(detail::stripTextDeclaration(decoded.getText()));
        entity.isLoaded = true;
    }

    // Текст параметрической сущности; внешняя загружается один раз и кэшируется в модели
    std::string_view parameterEntityText(DtdContext& context, std::string_view name)
    {
        EntityDeclaration* entity = context.model.findEntityForUpdate(name, true);

        if (entity == nullptr)
            throw DomException(DomErrorCode::PARSE_ERROR,
                "неизвестная параметрическая сущность %" + std::string(name) + ";");
        if (entity->isExternal && !entity->isLoaded)
            loadExternalEntityText(context.options, *entity);
        return entity->value;
    }
}
