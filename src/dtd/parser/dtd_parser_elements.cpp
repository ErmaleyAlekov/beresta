#include "dtd/dtd_parser.hpp"

#include <utility>

namespace beresta::dtd
{
    // <!ELEMENT имя спецификация-содержимого>
    void DtdParser::parseElementDeclaration()
    {
        ElementDeclaration declaration;

        _reader.advance(9);
        _reader.requireSpace(referencesInMarkup());
        declaration.name = _reader.readName();
        _reader.requireSpace(referencesInMarkup());
        parseContentSpec(declaration);
        _reader.skipSpaces(referencesInMarkup());
        _reader.expect(">");
        _reader.getContext().model.addElement(std::move(declaration));
    }

    // EMPTY | ANY | (#PCDATA ...) | модель из элементов
    void DtdParser::parseContentSpec(ElementDeclaration& declaration)
    {
        if (_reader.startsWith("EMPTY"))
        {
            _reader.advance(5);
            declaration.kind = ContentKind::EMPTY_CONTENT;
            return;
        }
        if (_reader.startsWith("ANY"))
        {
            _reader.advance(3);
            declaration.kind = ContentKind::ANY_CONTENT;
            return;
        }
        _reader.expect("(");
        _reader.skipSpaces(referencesInMarkup());
        if (_reader.startsWith("#PCDATA"))
        {
            parseMixed(declaration);
            return;
        }
        declaration.kind = ContentKind::CHILDREN;
        declaration.model = parseGroup(1);
    }

    /*
    ** Смешанное содержимое: (#PCDATA) или (#PCDATA | a | b)*.
    ** Если перечислены элементы, завершающая '*' обязательна.
    */
    void DtdParser::parseMixed(ElementDeclaration& declaration)
    {
        _reader.advance(7);
        declaration.kind = ContentKind::MIXED;
        while (true)
        {
            _reader.skipSpaces(referencesInMarkup());
            if (_reader.peek() != '|')
                break;
            _reader.advance(1);
            _reader.skipSpaces(referencesInMarkup());
            declaration.mixedNames.push_back(_reader.readName());
        }
        _reader.expect(")");
        if (!declaration.mixedNames.empty())
            _reader.expect("*");
        else if (_reader.peek() == '*')
            _reader.advance(1);
    }

    /*
    ** Группа после '(': частицы через ',' (последовательность) или через
    ** '|' (выбор). Смешивать разделители в одной группе запрещено.
    ** Глубина ограничена, чтобы злонамеренный DTD не переполнил стек.
    */
    content::Particle DtdParser::parseGroup(std::size_t depth)
    {
        content::Particle group;
        char separator = '\0';

        if (depth > 64)
            _reader.fail("слишком глубокая вложенность модели содержимого");
        group.children.push_back(parseContentParticle(depth));
        while (true)
        {
            _reader.skipSpaces(referencesInMarkup());
            char symbol = _reader.peek();
            if (symbol == ')')
                break;
            if ((symbol != ',' && symbol != '|') || (separator != '\0' && symbol != separator))
                _reader.fail("ожидался ')' или единый разделитель ',' / '|'");
            separator = symbol;
            _reader.advance(1);
            _reader.skipSpaces(referencesInMarkup());
            group.children.push_back(parseContentParticle(depth));
        }
        _reader.advance(1);
        if (separator == '|')
            group.kind = content::ParticleKind::CHOICE;
        parseOccurrence(group);
        return group;
    }

    // Частица: вложенная группа или имя элемента, затем необязательный ?, * или +
    content::Particle DtdParser::parseContentParticle(std::size_t depth)
    {
        content::Particle element;

        if (_reader.peek() == '(')
        {
            _reader.advance(1);
            _reader.skipSpaces(referencesInMarkup());
            return parseGroup(depth + 1);
        }
        element.kind = content::ParticleKind::ELEMENT;
        element.byQualifiedName = true;
        element.name = _reader.readName();
        parseOccurrence(element);
        return element;
    }
}
