#include "xsd/internal/identity_path.hpp"

namespace beresta::xsd::detail
{
    // Первый символ NCName: буква, '_' или байт UTF-8 вне ASCII (имя уже проверил движок XPath)
    // The first character of an NCName: a letter, '_' or a non-ASCII UTF-8 byte (the XPath engine has already checked the name)
    static bool isNameStart(unsigned char code) noexcept
    {
        return (code >= 'a' && code <= 'z') || (code >= 'A' && code <= 'Z') || code == '_' || code >= 0x80;
    }

    // Последующие символы NCName: ещё цифры, '.' и '-'
    // The following characters of an NCName: also digits, '.' and '-'
    static bool isNameChar(unsigned char code) noexcept
    {
        return isNameStart(code) || (code >= '0' && code <= '9') || code == '.' || code == '-';
    }

    /*
    ** NameTest ::= '*' | NCName ':' '*' | QName. Префикс заменяется URI по
    ** объявлениям документа схемы; неизвестный префикс — выражение не
    ** разобрать (движок XPath тоже не нашёл бы такие узлы).
    **
    ** NameTest ::= '*' | NCName ':' '*' | QName. The prefix is replaced with a URI by
    ** the declarations of the schema document; an unknown prefix means the expression cannot
    ** be parsed (the XPath engine would not find such nodes either).
    */
    bool PathReader::readNameTest(NameTest& test)
    {
        std::string_view first;

        skipSpaces();
        if (consume("*"))
            return true;
        if (!readName(first))
            return false;
        test.kind = NameTestKind::QUALIFIED;
        test.localName.assign(first.data(), first.size());
        if (!consume(":"))
            return true;
        auto declared = _context.namespaces.find(first);
        if (declared == _context.namespaces.end())
            return false;
        test.namespaceUri = declared->second;
        if (consume("*"))
        {
            test.kind = NameTestKind::NAMESPACE_ANY;
            return true;
        }
        std::string_view local;
        if (!readName(local))
            return false;
        test.localName.assign(local.data(), local.size());
        return true;
    }

    // NCName в текущей позиции (без двоеточия)
    // An NCName at the current position (without a colon)
    bool PathReader::readName(std::string_view& name) noexcept
    {
        std::size_t start = _position;

        if (_position >= _text.size() || !isNameStart(static_cast<unsigned char>(_text[_position])))
            return false;
        while (_position < _text.size() && isNameChar(static_cast<unsigned char>(_text[_position])))
            ++_position;
        name = _text.substr(start, _position - start);
        return true;
    }

    // Пробелы XML между лексемами допустимы
    // XML whitespace between tokens is allowed
    void PathReader::skipSpaces() noexcept
    {
        while (_position < _text.size() && (_text[_position] == ' ' || _text[_position] == '\t'
            || _text[_position] == '\n' || _text[_position] == '\r'))
            ++_position;
    }
}
