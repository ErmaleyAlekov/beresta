#include "parser/parse_session.hpp"
#include "parser/internal/attribute_value_decoder.hpp"

#include <utility>

namespace beresta::detail
{
    /*
    ** Открывающий тег <name ...> или пустой элемент <name .../>.
    ** Глубина ограничена настройкой maxDepth: это защищает рекурсивные
    ** алгоритмы (сериализацию, копирование, удаление) от огромных деревьев.
    */
    void ParseSession::parseStartTag()
    {
        XmlScanner& scanner = _inputs.current();

        if (_openElements.size() >= _options.maxDepth)
            scanner.failAt("превышена максимальная глубина вложенности ("
                + std::to_string(_options.maxDepth) + ")", _tokenStart, DomErrorCode::LIMIT_EXCEEDED);
        scanner.advance(1);
        std::string name(scanner.readName());
        std::vector<Attribute> attributes;
        parseAttributes(attributes);
        bool isEmpty = scanner.startsWith("/>");
        if (isEmpty)
            scanner.advance(2);
        else
            scanner.expect(">");
        emitStartElement(std::move(name), attributes, isEmpty);
    }

    /*
    ** Список атрибутов. Между атрибутами обязателен хотя бы один пробельный
    ** символ — любой: пробел, таб или перевод строки. Поэтому атрибуты,
    ** разнесённые по строкам, разбираются так же, как записанные в одну строку.
    */
    void ParseSession::parseAttributes(std::vector<Attribute>& attributes)
    {
        XmlScanner& scanner = _inputs.current();

        while (true)
        {
            std::size_t spaces = scanner.skipWhitespace();
            if (scanner.isAtEnd())
                scanner.fail("неожиданный конец ввода внутри тега");
            char symbol = scanner.peek();
            if (symbol == '>' || symbol == '/')
                return;
            if (spaces == 0)
                scanner.fail("атрибуты должны отделяться пробельным символом");
            parseAttribute(attributes);
        }
    }

    // Один атрибут: name = "value" или name = 'value'; значение сразу нормализуется
    void ParseSession::parseAttribute(std::vector<Attribute>& attributes)
    {
        XmlScanner& scanner = _inputs.current();
        std::size_t start = scanner.getOffset();
        std::string name(scanner.readName());

        scanner.skipWhitespace();
        scanner.expect("=");
        scanner.skipWhitespace();
        char quote = scanner.peek();
        if (quote != '"' && quote != '\'')
            scanner.fail("значение атрибута должно быть заключено в кавычки");
        scanner.advance(1);
        std::string_view raw = scanner.readUntil(std::string_view(&quote, 1));
        for (const Attribute& existing : attributes)
        {
            if (existing.name == name)
                scanner.failAt("повторный атрибут \"" + name + "\"", start);
        }
        std::string value = AttributeValueDecoder(_dtd.get(), _budget).decode(raw);
        attributes.push_back(Attribute{std::move(name), std::move(value)});
    }
}
