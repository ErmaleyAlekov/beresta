#include "dtd/dtd_parser.hpp"

namespace beresta::dtd
{
    // Суффикс повторения: ? (0..1), * (0..∞), + (1..∞)
    void DtdParser::parseOccurrence(content::Particle& particle)
    {
        char symbol = _reader.peek();

        if (symbol == '?')
            particle.maxOccurs = 1;
        else if (symbol == '*')
            particle.maxOccurs = content::UNBOUNDED;
        else if (symbol == '+')
            particle.maxOccurs = content::UNBOUNDED;
        else
            return;
        if (symbol != '+')
            particle.minOccurs = 0;
        _reader.advance(1);
    }

    // <!NOTATION имя SYSTEM "..." | PUBLIC "..." ["..."]>
    void DtdParser::parseNotationDeclaration()
    {
        _reader.advance(10);
        _reader.requireSpace(referencesInMarkup());
        std::string name = _reader.readName();
        _reader.requireSpace(referencesInMarkup());
        ExternalId id = _reader.readExternalId(true);
        if (id.systemId.empty() && id.publicId.empty())
            _reader.fail("в объявлении NOTATION ожидался SYSTEM или PUBLIC");
        _reader.skipSpaces(referencesInMarkup());
        _reader.expect(">");
        _reader.getContext().model.addNotation(name);
    }

    /*
    ** Условная секция <![INCLUDE[ ... ]]> или <![IGNORE[ ... ]]> — только во
    ** внешнем подмножестве. Ключевое слово часто задаётся параметрической
    ** сущностью (<![%draft;[ ... ]]>), поэтому ссылки здесь разрешены.
    */
    void DtdParser::parseConditionalSection()
    {
        if (!_reader.getContext().externalSubset)
            _reader.fail("условные секции допустимы только во внешнем подмножестве DTD");
        _reader.advance(3);
        _reader.skipSpaces(true);
        std::string keyword = _reader.readName();
        _reader.skipSpaces(true);
        _reader.expect("[");
        if (keyword == "INCLUDE")
            parseDeclarations(false, true);
        else if (keyword == "IGNORE")
            skipIgnoredSection();
        else
            _reader.fail("ожидалось INCLUDE или IGNORE");
    }

    // Пропуск IGNORE-секции с учётом вложенных <![ ... ]]>
    void DtdParser::skipIgnoredSection()
    {
        detail::XmlScanner& scanner = _reader.current();
        std::size_t depth = 1;

        while (depth > 0)
        {
            if (scanner.isAtEnd())
                scanner.fail("не закрыта секция IGNORE");
            if (scanner.startsWith("<!["))
            {
                ++depth;
                scanner.advance(3);
            }
            else if (scanner.startsWith("]]>"))
            {
                --depth;
                scanner.advance(3);
            }
            else
                scanner.advance(1);
        }
    }

    // Ссылки %pe; ВНУТРИ объявлений разрешены только во внешнем подмножестве
    bool DtdParser::referencesInMarkup() const noexcept
    {
        return _reader.getContext().externalSubset;
    }
}
