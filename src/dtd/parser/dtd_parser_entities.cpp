#include "dtd/dtd_parser.hpp"
#include "parser/internal/reference_decoding.hpp"
#include "utils/xml_chars.hpp"

#include <utility>

namespace beresta::dtd
{
    /*
    ** <!ENTITY name "значение"> | <!ENTITY name SYSTEM "uri" [NDATA нотация]>
    ** <!ENTITY % name ...> — параметрическая сущность.
    ** Переобъявление пяти предопределённых сущностей допускается и игнорируется.
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
        entity.name = _reader.readName();
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
    void DtdParser::parseExternalEntity(EntityDeclaration& entity)
    {
        ExternalId id = _reader.readExternalId(false);

        if (id.systemId.empty())
            _reader.fail("ожидалось значение сущности или SYSTEM / PUBLIC");
        entity.isExternal = true;
        entity.publicId = std::move(id.publicId);
        entity.systemId = std::move(id.systemId);
        std::size_t spaces = _reader.skipSpaces(referencesInMarkup());
        if (!_reader.startsWith("NDATA"))
            return;
        if (spaces == 0 || entity.isParameter)
            _reader.fail("NDATA допустим только у общей сущности после пробела");
        _reader.advance(5);
        _reader.requireSpace(referencesInMarkup());
        entity.notation = _reader.readName();
    }

    /*
    ** Текст замены внутренней сущности: символьные ссылки и %pe;
    ** раскрываются сразу, а ссылки на общие сущности (&x;) остаются
    ** как есть — они раскроются в момент использования (раздел 4.5).
    */
    std::string DtdParser::parseEntityValue()
    {
        std::string literal = _reader.readQuoted();
        std::string value;
        std::size_t position = 0;

        while (position < literal.size())
        {
            std::size_t found = literal.find_first_of("&%", position);
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
    std::size_t DtdParser::appendValueReference(std::string_view literal, std::size_t index,
        std::string& value)
    {
        std::size_t semicolon = literal.find(';', index);

        if (semicolon == std::string_view::npos)
            _reader.fail("ссылка в значении сущности не завершена символом ';'");
        std::string_view name = literal.substr(index + 1, semicolon - index - 1);
        if (literal[index] == '%')
        {
            if (!referencesInMarkup())
                _reader.fail("ссылки %pe; внутри объявлений запрещены во внутреннем подмножестве");
            std::string_view text = parameterEntityText(_reader.getContext(), name);
            _reader.getContext().budget.charge(text.size(), 1);
            value.append(text);
        }
        else if (!name.empty() && name[0] == '#')
        {
            if (!detail::decodeCharacterReference(name.substr(1), value))
                _reader.fail("недопустимая символьная ссылка &" + std::string(name) + ";");
        }
        else if (utils::isValidName(name))
            value.append(literal.substr(index, semicolon - index + 1));
        else
            _reader.fail("недопустимая ссылка в значении сущности");
        return semicolon + 1;
    }
}
