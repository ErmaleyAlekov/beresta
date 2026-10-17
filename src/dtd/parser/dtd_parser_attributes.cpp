#include "dtd/dtd_parser.hpp"
#include "dtd/attribute_defaults.hpp"

#include <utility>

namespace beresta::dtd
{
    // <!ATTLIST элемент (атрибут тип умолчание)*>
    void DtdParser::parseAttlistDeclaration()
    {
        _reader.advance(9);
        _reader.requireSpace(referencesInMarkup());
        std::string element = _reader.readName();
        while (true)
        {
            std::size_t spaces = _reader.skipSpaces(referencesInMarkup());
            if (_reader.peek() == '>')
            {
                _reader.advance(1);
                return;
            }
            if (spaces == 0)
                _reader.fail("ожидался пробел перед определением атрибута");
            parseAttributeDefinition(element);
        }
    }

    // Одно определение: имя, тип, значение по умолчанию
    void DtdParser::parseAttributeDefinition(const std::string& element)
    {
        AttributeDeclaration declaration;

        declaration.name = _reader.readName();
        _reader.requireSpace(referencesInMarkup());
        parseAttributeType(declaration);
        _reader.requireSpace(referencesInMarkup());
        parseDefaultDeclaration(declaration);
        _reader.getContext().model.addAttribute(element, std::move(declaration));
    }

    // Ключевое слово типа атрибута и соответствующий тип
    struct TypeKeyword
    {
        const char* text;
        AttributeType type;
    };

    /*
    ** Ключевые слова упорядочены от длинных к коротким: IDREFS раньше
    ** IDREF раньше ID, иначе "IDREFS" распознался бы как "ID".
    */
    static const TypeKeyword TYPE_KEYWORDS[] = {
        {"CDATA", AttributeType::CDATA}, {"IDREFS", AttributeType::IDREFS},
        {"IDREF", AttributeType::IDREF}, {"ID", AttributeType::ID},
        {"ENTITIES", AttributeType::ENTITIES}, {"ENTITY", AttributeType::ENTITY},
        {"NMTOKENS", AttributeType::NMTOKENS}, {"NMTOKEN", AttributeType::NMTOKEN},
        {"NOTATION", AttributeType::NOTATION}
    };

    // Тип атрибута: ключевое слово, NOTATION (имена) или перечисление (a | b)
    void DtdParser::parseAttributeType(AttributeDeclaration& declaration)
    {
        for (const TypeKeyword& keyword : TYPE_KEYWORDS)
        {
            if (!_reader.startsWith(keyword.text))
                continue;
            _reader.advance(std::string_view(keyword.text).size());
            declaration.type = keyword.type;
            if (keyword.type == AttributeType::NOTATION)
            {
                _reader.requireSpace(referencesInMarkup());
                parseEnumeration(declaration, true);
            }
            return;
        }
        declaration.type = AttributeType::ENUMERATION;
        parseEnumeration(declaration, false);
    }

    // (a | b | c) — имена нотаций либо Nmtoken-значения
    void DtdParser::parseEnumeration(AttributeDeclaration& declaration, bool names)
    {
        _reader.expect("(");
        while (true)
        {
            _reader.skipSpaces(referencesInMarkup());
            if (names)
                declaration.allowedValues.push_back(_reader.readName());
            else
                declaration.allowedValues.push_back(_reader.readNmtoken());
            _reader.skipSpaces(referencesInMarkup());
            if (_reader.peek() != '|')
                break;
            _reader.advance(1);
        }
        _reader.expect(")");
    }

    /*
    ** #REQUIRED | #IMPLIED | [#FIXED] "значение". Значение по умолчанию
    ** сразу нормализуется так же, как значение в документе, чтобы
    ** подставлять его без повторной обработки.
    */
    void DtdParser::parseDefaultDeclaration(AttributeDeclaration& declaration)
    {
        if (_reader.startsWith("#REQUIRED"))
        {
            _reader.advance(9);
            declaration.defaultKind = DefaultKind::REQUIRED;
            return;
        }
        if (_reader.startsWith("#IMPLIED"))
        {
            _reader.advance(8);
            declaration.defaultKind = DefaultKind::IMPLIED;
            return;
        }
        declaration.defaultKind = DefaultKind::VALUE;
        if (_reader.startsWith("#FIXED"))
        {
            _reader.advance(6);
            _reader.requireSpace(referencesInMarkup());
            declaration.defaultKind = DefaultKind::FIXED;
        }
        declaration.defaultValue = normalizeDefaultValue(_reader.getContext(),
            _reader.readQuoted(), declaration.type);
    }
}
