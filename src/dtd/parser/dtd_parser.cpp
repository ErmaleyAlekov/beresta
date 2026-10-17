#include "dtd/dtd_parser.hpp"

namespace beresta::dtd
{
    // Парсер работает поверх лексического читателя
    DtdParser::DtdParser(DtdReader& reader) noexcept
        : _reader(reader)
    {
    }

    // Внутреннее подмножество заканчивается ']'
    void DtdParser::parseInternalSubset()
    {
        parseDeclarations(true, false);
    }

    // Внешнее подмножество читается до конца
    void DtdParser::parseExternalSubset()
    {
        parseDeclarations(false, false);
    }

    /*
    ** Цикл объявлений. Между объявлениями ссылки %pe; разрешены всегда.
    ** Условием выхода служит ']' (внутреннее подмножество), "]]>"
    ** (конец секции INCLUDE) или конец текста (внешнее подмножество).
    */
    void DtdParser::parseDeclarations(bool stopAtBracket, bool stopAtSectionEnd)
    {
        while (true)
        {
            _reader.skipSpaces(true);
            if (_reader.atEnd())
            {
                if (stopAtBracket || stopAtSectionEnd)
                    _reader.fail("неожиданный конец DTD");
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

    // Выбор объявления по ключевому слову
    void DtdParser::parseMarkupDeclaration()
    {
        if (_reader.startsWith("<!ENTITY"))
            parseEntityDeclaration();
        else if (_reader.startsWith("<!ELEMENT"))
            parseElementDeclaration();
        else if (_reader.startsWith("<!ATTLIST"))
            parseAttlistDeclaration();
        else if (_reader.startsWith("<!NOTATION"))
            parseNotationDeclaration();
        else if (_reader.startsWith("<!--"))
        {
            _reader.advance(4);
            _reader.current().readUntil("-->");
        }
        else if (_reader.startsWith("<?"))
        {
            _reader.advance(2);
            _reader.current().readUntil("?>");
        }
        else if (_reader.startsWith("<!["))
            parseConditionalSection();
        else
            _reader.fail("неизвестное объявление в DTD");
    }
}
