#include "parser/parse_session.hpp"
#include "dtd/dtd_parser.hpp"
#include "encoding/encoding_detector.hpp"
#include "exception/dom_exception.hpp"
#include "parser/internal/reference_decoding.hpp"
#include "parser/resource_resolver.hpp"

namespace beresta::detail
{
    /*
    ** <!DOCTYPE имя [SYSTEM "..." | PUBLIC "..." "..."] [[внутреннее подмножество]]>
    ** DOCTYPE допустим один раз, до корня и только в основном документе.
    */
    void ParseSession::parseDoctype()
    {
        XmlScanner& scanner = _inputs.current();

        if (_inputs.inEntity() || _rootSeen || _doctypeSeen)
            scanner.failAt("DOCTYPE допустим один раз и только до корневого элемента", _tokenStart);
        _doctypeSeen = true;
        _dtd = std::make_shared<dtd::DtdModel>();
        dtd::DtdContext context{*_dtd, _options, _budget, false};
        dtd::DtdReader reader(scanner, context);
        scanner.advance(9);
        reader.requireSpace(false);
        std::string name = reader.readName();
        reader.skipSpaces(false);
        dtd::ExternalId id = reader.readExternalId(false);
        reader.skipSpaces(false);
        std::string subset;
        if (scanner.peek() == '[')
            parseInternalSubset(reader, subset);
        scanner.expect(">");
        if (!id.systemId.empty() && _options.loadExternalDtd)
            loadExternalSubset(id);
        announceDoctype(name, id, subset);
    }

    // Объявления между '[' и ']'; их дословный текст сохраняется для сериализации
    void ParseSession::parseInternalSubset(dtd::DtdReader& reader, std::string& subset)
    {
        XmlScanner& scanner = _inputs.current();

        scanner.advance(1);
        std::size_t begin = scanner.getOffset();
        dtd::DtdParser(reader).parseInternalSubset();
        subset = std::string(scanner.slice(begin, scanner.getOffset()));
        scanner.expect("]");
        reader.skipSpaces(false);
    }

    /*
    ** Внешнее подмножество загружается через резолвер. Объявления
    ** внутреннего подмножества уже в модели и имеют приоритет: при
    ** повторном объявлении «выигрывает» первое (так требует стандарт).
    ** Ошибка внутри DTD-файла сообщается с его собственными координатами.
    */
    void ParseSession::loadExternalSubset(const dtd::ExternalId& id)
    {
        if (_options.resolver == nullptr)
            throw DomException(DomErrorCode::RESOURCE_ERROR,
                "для загрузки внешнего DTD нужен ResourceResolver");
        ResourceRequest request{ResourceKind::EXTERNAL_DTD, id.publicId, id.systemId, _options.baseUri};
        std::optional<Resource> resource = _options.resolver->resolve(request);
        if (!resource.has_value())
            throw DomException(DomErrorCode::RESOURCE_ERROR,
                "не удалось загрузить внешний DTD \"" + id.systemId + "\"");
        encoding::DecodedDocument decoded = encoding::decodeDocument(resource->content, "");
        XmlScanner scanner(stripTextDeclaration(decoded.getText()));
        dtd::DtdContext context{*_dtd, _options, _budget, true};
        dtd::DtdReader reader(scanner, context);
        try
        {
            dtd::DtdParser(reader).parseExternalSubset();
        }
        catch (const ParseException& problem)
        {
            throw DomException(problem.getCode(), "во внешнем DTD \"" + id.systemId + "\", "
                + problem.getMessage());
        }
    }

    // Событие DOCTYPE с моделью; при включённой проверке создаётся валидатор
    void ParseSession::announceDoctype(const std::string& name, const dtd::ExternalId& id,
        const std::string& subset)
    {
        DocumentType node(name, id.publicId, id.systemId, subset);

        node.setModel(_dtd);
        if (_options.validateDtd)
            _validator = std::make_unique<dtd::DtdValidator>(*_dtd, name, *this);
        markEvent();
        _handler.documentType(node);
    }
}
