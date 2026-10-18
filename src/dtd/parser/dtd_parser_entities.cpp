#include "dtd/dtd_parser.hpp"
#include "i18n/i18n.hpp"
#include "parser/internal/reference_decoding.hpp"
#include "utils/string_utils.hpp"
#include "utils/xml_chars.hpp"

#include <utility>

namespace beresta::dtd
{
    /*
    ** <!ENTITY name "значение"> | <!ENTITY name SYSTEM "uri" [NDATA нотация]>
    ** <!ENTITY % name ...> — параметрическая сущность.
    ** Переобъявление пяти предопределённых сущностей допускается и игнорируется.
    **
    ** <!ENTITY name "value"> | <!ENTITY name SYSTEM "uri" [NDATA notation]>
    ** <!ENTITY % name ...> — a parameter entity.
    ** Redeclaring the five predefined entities is allowed and ignored.
    */
    void DtdParser::parseEntityDeclaration()
    {
        EntityDeclaration entity;

        _reader.advance(8);
        _reader.requireSpace(referencesInMarkup());
        if (_reader.peek() == '%')
        {
            _reader.advance(1);
            _reader.requireSpace(referencesInMarkup());
            entity.isParameter = true;
        }
        entity.name = readDeclaredName();
        entity.externallyDeclared = _reader.inExternalText();
        _reader.requireSpace(referencesInMarkup());
        if (_reader.peek() == '"' || _reader.peek() == '\'')
            entity.value = parseEntityValue();
        else
            parseExternalEntity(entity);
        _reader.skipSpaces(referencesInMarkup());
        _reader.expect(">");
        if (!entity.isParameter && detail::predefinedEntity(entity.name).has_value())
            return;
        _reader.getContext().model.addEntity(std::move(entity));
    }

    // Внешняя сущность: идентификатор и, для общих сущностей, необязательный NDATA
    // An external entity: the identifier and, for general entities, an optional NDATA
    void DtdParser::parseExternalEntity(EntityDeclaration& entity)
    {
        ExternalId id = _reader.readExternalId(false);

        if (!id.present)
            _reader.fail(i18n::format(i18n::Message::DTD_ENTITY_VALUE_EXPECTED));
        entity.isExternal = true;
        // Относительный systemId разрешается от текста, где стоит объявление
        // A relative systemId is resolved against the text containing the declaration
        entity.baseUri = _reader.currentBaseUri();
        entity.publicId = std::move(id.publicId);
        entity.systemId = std::move(id.systemId);
        std::size_t spaces = _reader.skipSpaces(referencesInMarkup());
        if (!_reader.startsWith("NDATA"))
            return;
        if (spaces == 0 || entity.isParameter)
            _reader.fail(i18n::format(i18n::Message::DTD_NDATA_PLACEMENT));
        _reader.advance(5);
        _reader.requireSpace(referencesInMarkup());
        entity.notation = _reader.readName();
    }

    /*
    ** Текст замены внутренней сущности: символьные ссылки и %pe;
    ** раскрываются сразу, а ссылки на общие сущности (&x;) остаются
    ** как есть — они раскроются в момент использования (раздел 4.5).
    **
    ** The replacement text of an internal entity: character references and %pe;
    ** are expanded at once, while references to general entities (&x;) stay
    ** as is — they are expanded at the point of use (section 4.5).
    */
    std::string DtdParser::parseEntityValue()
    {
        std::string literal = _reader.readQuoted();
        std::string value;
        std::size_t position = 0;

        while (position < literal.size())
        {
            std::size_t found = utils::findAny(literal, position, "&%");
            if (found == std::string::npos)
                found = literal.size();
            value.append(literal, position, found - position);
            if (found == literal.size())
                break;
            position = appendValueReference(literal, found, value);
        }
        return value;
    }

    // Обработка одной ссылки внутри значения сущности; возвращает позицию после ';'
    // Processes one reference inside an entity value; returns the position after ';'
    std::size_t DtdParser::appendValueReference(std::string_view literal, std::size_t index,
        std::string& value)
    {
        std::size_t semicolon = literal.find(';', index);

        if (semicolon == std::string_view::npos)
            _reader.fail(i18n::format(i18n::Message::DTD_VALUE_REFERENCE_UNTERMINATED));
        std::string_view name = literal.substr(index + 1, semicolon - index - 1);
        if (literal[index] == '%')
        {
            if (!referencesInMarkup())
                _reader.fail(i18n::format(i18n::Message::DTD_PE_IN_INTERNAL_MARKUP));
            std::string_view text = parameterEntityText(_reader.getContext(), name);
            _reader.getContext().budget.charge(text.size(), 1);
            value.append(text);
        }
        else if (!name.empty() && name[0] == '#')
        {
            if (!detail::decodeCharacterReference(name.substr(1), value))
                _reader.fail(i18n::format(i18n::Message::CHARACTER_REFERENCE_INVALID, {name}));
        }
        else if (utils::isValidName(name))
            value.append(literal.substr(index, semicolon - index + 1));
        else
            _reader.fail(i18n::format(i18n::Message::DTD_VALUE_REFERENCE_INVALID));
        return semicolon + 1;
    }
}
