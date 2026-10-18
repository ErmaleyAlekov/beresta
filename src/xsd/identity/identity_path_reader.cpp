#include "xsd/internal/identity_path.hpp"

namespace beresta::xsd::detail
{
    // Читатель стоит в начале текста выражения
    // The reader stands at the start of the expression text
    PathReader::PathReader(std::string_view text, const xpath::XPathContext& context) noexcept
        : _text(text), _position(0), _context(context)
    {
    }

    // Selector / Field ::= Path ( '|' Path )* — и ничего после последнего варианта
    // Selector / Field ::= Path ( '|' Path )* — and nothing after the last alternative
    bool PathReader::readAll(std::vector<StreamPath>& paths)
    {
        do
        {
            StreamPath path;
            if (!readPath(path))
                return false;
            paths.push_back(path);
            skipSpaces();
        }
        while (consume("|"));
        return _position == _text.size();
    }

    /*
    ** Path ::= ('.//')? Step ( '/' Step )* [ '/' '@' NameTest ]. Двойная
    ** косая черта допустима только в самом начале; атрибут — только
    ** последним шагом.
    **
    ** Path ::= ('.//')? Step ( '/' Step )* [ '/' '@' NameTest ]. A double
    ** slash is allowed only at the very start; an attribute — only as the
    ** last step.
    */
    bool PathReader::readPath(StreamPath& path)
    {
        skipSpaces();
        if (consume(".//"))
            path.descendant = true;
        while (true)
        {
            bool attribute = false;
            if (!readStep(path, attribute))
                return false;
            skipSpaces();
            if (attribute || !consume("/"))
                return true;
            if (_position < _text.size() && _text[_position] == '/')
                return false;
        }
    }

    // Step ::= '.' | ('child::')? NameTest | ('attribute::' | '@') NameTest
    // Step ::= '.' | ('child::')? NameTest | ('attribute::' | '@') NameTest
    bool PathReader::readStep(StreamPath& path, bool& attribute)
    {
        skipSpaces();
        if (consume("attribute::") || consume("@"))
        {
            attribute = true;
            path.attribute = true;
            return readNameTest(path.attributeTest);
        }
        if (consume("child::"))
            skipSpaces();
        bool self = _position < _text.size() && _text[_position] == '.';
        bool nameFollows = _position + 1 < _text.size() && _text[_position + 1] != '/'
            && _text[_position + 1] != '|' && _text[_position + 1] != ' ';
        if (self && !nameFollows)
        {
            ++_position;
            return true;
        }
        NameTest test;
        if (!readNameTest(test))
            return false;
        path.steps.push_back(test);
        return true;
    }

    // Совпадает ли текст с token в текущей позиции; при совпадении он пропускается
    // Whether the text matches token at the current position; on a match it is skipped
    bool PathReader::consume(std::string_view token) noexcept
    {
        if (_text.substr(_position, token.size()) != token)
            return false;
        _position += token.size();
        return true;
    }
}
