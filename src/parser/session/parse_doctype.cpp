#include "parser/parse_session.hpp"
#include "i18n/i18n.hpp"
#include "dtd/dtd_declaration_checks.hpp"
#include "dtd/dtd_parser.hpp"
#include "encoding/encoding_detector.hpp"
#include "exception/dom_exception.hpp"
#include "parser/internal/declaration_reader.hpp"
#include "parser/resource_resolver.hpp"

namespace beresta::detail
{
    /*
    ** <!DOCTYPE имя [SYSTEM "..." | PUBLIC "..." "..."] [[внутреннее подмножество]]>
    ** DOCTYPE допустим один раз, до корня и только в основном документе.
    **
    ** <!DOCTYPE name [SYSTEM "..." | PUBLIC "..." "..."] [[internal subset]]>
    ** A DOCTYPE is allowed once, before the root and only in the main document.
    */
    void ParseSession::parseDoctype()
    {
        XmlScanner& scanner = _inputs.current();

        if (_inputs.inEntity() || _rootSeen || _doctypeSeen)
            scanner.failAt(i18n::format(i18n::Message::DOCTYPE_PLACEMENT), _tokenStart);
        _doctypeSeen = true;
        _dtd = std::make_shared<dtd::DtdModel>();
        dtd::DtdContext context{*_dtd, _options, _budget, false, _options.baseUri, &_handler};
        dtd::DtdReader reader(scanner, context);
        scanner.advance(9);
        reader.requireSpace(false);
        std::string name = reader.readName();
        reader.skipSpaces(false);
        dtd::ExternalId id = reader.readExternalId(false);
        reader.skipSpaces(false);
        // Внешнее подмножество (даже незагруженное) меняет правила необъявленных сущностей
        // An external subset (even one not loaded) changes the rules for undeclared entities
        if (id.present)
            _dtd->markExternalParts();
        std::string subset;
        if (scanner.peek() == '[')
            parseInternalSubset(reader, subset);
        scanner.expect(">");
        if (!id.systemId.empty() && _options.loadExternalDtd)
            loadExternalSubset(id);
        announceDoctype(name, id, subset);
    }

    // Объявления между '[' и ']'; их дословный текст сохраняется для сериализации
    // The declarations between '[' and ']'; their verbatim text is kept for serialization
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
    **
    ** The external subset is loaded through the resolver. The declarations of
    ** the internal subset are already in the model and take priority: on a
    ** repeated declaration the first one "wins" (the standard requires it).
    ** An error inside the DTD file is reported with its own coordinates.
    */
    void ParseSession::loadExternalSubset(const dtd::ExternalId& id)
    {
        if (_options.resolver == nullptr)
            throw DomException(DomErrorCode::RESOURCE_ERROR,
                i18n::format(i18n::Message::EXTERNAL_DTD_RESOLVER));
        ResourceRequest request{ResourceKind::EXTERNAL_DTD, id.publicId, id.systemId, _options.baseUri};
        std::optional<Resource> resource = _options.resolver->resolve(request);
        if (!resource.has_value())
            throw DomException(DomErrorCode::RESOURCE_ERROR,
                i18n::format(i18n::Message::EXTERNAL_DTD_LOAD, {id.systemId}));
        encoding::DecodedDocument decoded = encoding::decodeDocument(resource->content, "");
        XmlScanner scanner(stripTextDeclaration(decoded.getText()));
        dtd::DtdContext context{*_dtd, _options, _budget, true, resource->uri, &_handler};
        dtd::DtdReader reader(scanner, context);
        try
        {
            dtd::DtdParser(reader).parseExternalSubset();
        }
        catch (const ParseException& problem)
        {
            throw DomException(problem.getCode(), i18n::format(i18n::Message::IN_EXTERNAL_DTD, {id.systemId, problem.getMessage()}));
        }
    }

    // Событие DOCTYPE с моделью; при включённой проверке создаётся валидатор
    // The DOCTYPE event with the model; with validation enabled a validator is created
    void ParseSession::announceDoctype(const std::string& name, const dtd::ExternalId& id,
        const std::string& subset)
    {
        DocumentType node(name, id.publicId, id.systemId, subset);

        node.setModel(_dtd);
        markEvent();
        if (_options.validateDtd)
        {
            _validator = std::make_unique<dtd::DtdValidator>(*_dtd, name, *this,
                _options.namespaceAware, _standalone);
            dtd::checkDeclarations(*_dtd, *this);
        }
        _handler.documentType(node);
    }
}
