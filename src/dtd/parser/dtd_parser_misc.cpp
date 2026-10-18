#include "dtd/dtd_parser.hpp"
#include "i18n/i18n.hpp"

namespace beresta::dtd
{
    // Суффикс повторения: ? (0..1), * (0..∞), + (1..∞)
    // The repetition suffix: ? (0..1), * (0..∞), + (1..∞)
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
    // <!NOTATION name SYSTEM "..." | PUBLIC "..." ["..."]>
    void DtdParser::parseNotationDeclaration()
    {
        _reader.advance(10);
        _reader.requireSpace(referencesInMarkup());
        std::string name = readDeclaredName();
        _reader.requireSpace(referencesInMarkup());
        ExternalId id = _reader.readExternalId(true);
        if (!id.present)
            _reader.fail(i18n::format(i18n::Message::DTD_NOTATION_ID));
        _reader.skipSpaces(referencesInMarkup());
        _reader.expect(">");
        _reader.getContext().model.addNotation(name);
    }

    /*
    ** Условная секция <![INCLUDE[ ... ]]> или <![IGNORE[ ... ]]> — только во
    ** внешнем подмножестве. Ключевое слово часто задаётся параметрической
    ** сущностью (<![%draft;[ ... ]]>), поэтому ссылки здесь разрешены.
    **
    ** A conditional section <![INCLUDE[ ... ]]> or <![IGNORE[ ... ]]> — only in
    ** the external subset. The keyword is often given by a parameter
    ** entity (<![%draft;[ ... ]]>), so references are allowed here.
    */
    void DtdParser::parseConditionalSection()
    {
        std::size_t startFrame = _reader.frameId();

        if (!_reader.inExternalText())
            _reader.fail(i18n::format(i18n::Message::DTD_CONDITIONAL_INTERNAL));
        _reader.advance(3);
        _reader.skipSpaces(true);
        std::string keyword = _reader.readName();
        _reader.skipSpaces(true);
        _reader.expect("[");
        // VC: Proper Conditional Section/PE Nesting — "<![" и "[" в одном тексте
        // VC: Proper Conditional Section/PE Nesting — "<![" and "[" in the same text
        if (_reader.frameId() != startFrame)
            _reader.getContext().model.addValidityNote(
                i18n::format(i18n::Message::DTD_CONDITIONAL_SPLIT_PE));
        if (keyword == "INCLUDE")
            parseDeclarations(false, true);
        else if (keyword == "IGNORE")
            skipIgnoredSection();
        else
            _reader.fail(i18n::format(i18n::Message::DTD_INCLUDE_OR_IGNORE));
    }

    // Пропуск IGNORE-секции с учётом вложенных <![ ... ]]>
    // Skips an IGNORE section taking nested <![ ... ]]> into account
    void DtdParser::skipIgnoredSection()
    {
        detail::XmlScanner& scanner = _reader.current();
        std::size_t depth = 1;

        while (depth > 0)
        {
            if (scanner.isAtEnd())
                scanner.fail(i18n::format(i18n::Message::DTD_IGNORE_UNCLOSED));
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

    // Ссылки %pe; ВНУТРИ объявлений разрешены во внешнем подмножестве и во внешних PE
    // %pe; references INSIDE declarations are allowed in the external subset and in external PEs
    bool DtdParser::referencesInMarkup() const noexcept
    {
        return _reader.inExternalText();
    }
}
