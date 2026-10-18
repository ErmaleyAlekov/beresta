#include "dtd/dtd_parser.hpp"
#include "i18n/i18n.hpp"

namespace beresta::dtd
{
    // Парсер работает поверх лексического читателя
    // The parser works on top of the lexical reader
    DtdParser::DtdParser(DtdReader& reader) noexcept
        : _reader(reader)
    {
    }

    // Внутреннее подмножество заканчивается ']'
    // The internal subset ends with ']'
    void DtdParser::parseInternalSubset()
    {
        parseDeclarations(true, false);
    }

    // Внешнее подмножество читается до конца
    // The external subset is read to the end
    void DtdParser::parseExternalSubset()
    {
        parseDeclarations(false, false);
    }

    /*
    ** Цикл объявлений. Между объявлениями ссылки %pe; разрешены всегда.
    ** Условием выхода служит ']' (внутреннее подмножество), "]]>"
    ** (конец секции INCLUDE) или конец текста (внешнее подмножество).
    **
    ** The declaration loop. Between declarations %pe; references are always allowed.
    ** The exit condition is ']' (the internal subset), "]]>"
    ** (the end of an INCLUDE section) or the end of the text (the external subset).
    */
    void DtdParser::parseDeclarations(bool stopAtBracket, bool stopAtSectionEnd)
    {
        while (true)
        {
            _reader.skipSpaces(true);
            if (_reader.atEnd())
            {
                if (stopAtBracket || stopAtSectionEnd)
                    _reader.fail(i18n::format(i18n::Message::DTD_UNEXPECTED_END));
                return;
            }
            if (stopAtBracket && _reader.peek() == ']')
                return;
            if (stopAtSectionEnd && _reader.startsWith("]]>"))
            {
                _reader.advance(3);
                return;
            }
            parseMarkupDeclaration();
        }
    }

    /*
    ** Выбор объявления по ключевому слову. Номер текста, в котором
    ** объявление началось, запоминается: конец обязан быть там же.
    **
    ** Chooses a declaration by keyword. The number of the text in which
    ** the declaration started is remembered: the end must be in the same text.
    */
    void DtdParser::parseMarkupDeclaration()
    {
        std::size_t startFrame = _reader.frameId();

        if (_reader.startsWith("<!ENTITY"))
            parseEntityDeclaration();
        else if (_reader.startsWith("<!ELEMENT"))
            parseElementDeclaration();
        else if (_reader.startsWith("<!ATTLIST"))
            parseAttlistDeclaration();
        else if (_reader.startsWith("<!NOTATION"))
            parseNotationDeclaration();
        else if (_reader.startsWith("<!--"))
            parseComment();
        else if (_reader.startsWith("<?"))
            parseProcessingInstruction();
        else if (_reader.startsWith("<!["))
            parseConditionalSection();
        else
            _reader.fail(i18n::format(i18n::Message::DTD_UNKNOWN_DECLARATION));
        checkDeclarationNesting(startFrame);
    }
}
