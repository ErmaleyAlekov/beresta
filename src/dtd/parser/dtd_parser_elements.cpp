#include "dtd/dtd_parser.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::dtd
{
    // <!ELEMENT имя спецификация-содержимого>
    // <!ELEMENT name content-specification>
    void DtdParser::parseElementDeclaration()
    {
        ElementDeclaration declaration;

        _reader.advance(9);
        _reader.requireSpace(referencesInMarkup());
        declaration.name = _reader.readName();
        declaration.externallyDeclared = _reader.inExternalText();
        _reader.requireSpace(referencesInMarkup());
        parseContentSpec(declaration);
        _reader.skipSpaces(referencesInMarkup());
        _reader.expect(">");
        std::string name = declaration.name;
        // VC: Unique Element Type Declaration — повторное объявление (первое остаётся в силе)
        // VC: Unique Element Type Declaration — a repeated declaration (the first one stays in force)
        if (!_reader.getContext().model.addElement(std::move(declaration)))
            _reader.getContext().model.addValidityNote(i18n::format(i18n::Message::DTD_ELEMENT_REDECLARED, {name}));
    }

    // EMPTY | ANY | (#PCDATA ...) | модель из элементов
    // EMPTY | ANY | (#PCDATA ...) | a model of elements
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
        std::size_t openFrame = _reader.frameId();
        _reader.skipSpaces(referencesInMarkup());
        if (_reader.startsWith("#PCDATA"))
        {
            parseMixed(declaration, openFrame);
            return;
        }
        declaration.kind = ContentKind::CHILDREN;
        declaration.model = parseGroup(1, openFrame);
    }

    /*
    ** Смешанное содержимое: (#PCDATA) или (#PCDATA | a | b)*.
    ** Если перечислены элементы, завершающая '*' обязательна.
    **
    ** Mixed content: (#PCDATA) or (#PCDATA | a | b)*.
    ** If elements are listed, the trailing '*' is mandatory.
    */
    void DtdParser::parseMixed(ElementDeclaration& declaration, std::size_t openFrame)
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
        checkGroupNesting(openFrame);
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
    **
    ** A group after '(': particles separated by ',' (a sequence) or by
    ** '|' (a choice). Mixing separators in one group is forbidden.
    ** The depth is limited so that a malicious DTD cannot overflow the stack.
    */
    content::Particle DtdParser::parseGroup(std::size_t depth, std::size_t openFrame)
    {
        content::Particle group;
        char separator = '\0';

        if (depth > 64)
            _reader.fail(i18n::format(i18n::Message::MODEL_TOO_DEEP));
        group.children.push_back(parseContentParticle(depth));
        while (true)
        {
            _reader.skipSpaces(referencesInMarkup());
            char symbol = _reader.peek();
            if (symbol == ')')
                break;
            if ((symbol != ',' && symbol != '|') || (separator != '\0' && symbol != separator))
                _reader.fail(i18n::format(i18n::Message::DTD_GROUP_SEPARATOR));
            separator = symbol;
            _reader.advance(1);
            _reader.skipSpaces(referencesInMarkup());
            group.children.push_back(parseContentParticle(depth));
        }
        checkGroupNesting(openFrame);
        _reader.advance(1);
        if (separator == '|')
            group.kind = content::ParticleKind::CHOICE;
        parseOccurrence(group);
        return group;
    }

    // Частица: вложенная группа или имя элемента, затем необязательный ?, * или +
    // A particle: a nested group or an element name, then an optional ?, * or +
    content::Particle DtdParser::parseContentParticle(std::size_t depth)
    {
        content::Particle element;

        if (_reader.peek() == '(')
        {
            _reader.advance(1);
            std::size_t openFrame = _reader.frameId();
            _reader.skipSpaces(referencesInMarkup());
            return parseGroup(depth + 1, openFrame);
        }
        element.kind = content::ParticleKind::ELEMENT;
        element.byQualifiedName = true;
        element.name = _reader.readName();
        parseOccurrence(element);
        return element;
    }
}
