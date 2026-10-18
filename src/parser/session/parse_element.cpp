#include "parser/parse_session.hpp"
#include "i18n/i18n.hpp"
#include "parser/internal/attribute_value_decoder.hpp"

#include <utility>

namespace beresta::detail
{
    /*
    ** Открывающий тег <name ...> или пустой элемент <name .../>.
    ** Глубина ограничена настройкой maxDepth: это защита от злонамеренно
    ** глубоких документов (сами обходы дерева стек вызовов не расходуют).
    **
    ** A start tag <name ...> or an empty element <name .../>.
    ** The depth is limited by the maxDepth setting: this protects against maliciously
    ** deep documents (the tree traversals themselves do not use the call stack).
    */
    void ParseSession::parseStartTag()
    {
        XmlScanner& scanner = _inputs.current();

        if (_openElements.size() >= _options.maxDepth)
            scanner.failAt(i18n::format(i18n::Message::LIMIT_DEPTH, {std::to_string(_options.maxDepth)}), _tokenStart, DomErrorCode::LIMIT_EXCEEDED);
        scanner.advance(1);
        std::string name(scanner.readName());
        std::vector<Attribute>& attributes = _attributeBuffer;
        attributes.clear();
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
    **
    ** The attribute list. At least one whitespace character is mandatory between
    ** attributes — any: a space, a tab or a line break. That is why attributes
    ** spread over several lines are parsed the same way as ones written on one line.
    */
    void ParseSession::parseAttributes(std::vector<Attribute>& attributes)
    {
        XmlScanner& scanner = _inputs.current();

        while (true)
        {
            std::size_t spaces = scanner.skipWhitespace();
            if (scanner.isAtEnd())
                scanner.fail(i18n::format(i18n::Message::UNEXPECTED_END_IN_TAG));
            char symbol = scanner.peek();
            if (symbol == '>' || symbol == '/')
                return;
            if (spaces == 0)
                scanner.fail(i18n::format(i18n::Message::ATTRIBUTES_NOT_SEPARATED));
            parseAttribute(attributes);
        }
    }

    // Один атрибут: name = "value" или name = 'value'; значение сразу нормализуется
    // One attribute: name = "value" or name = 'value'; the value is normalized at once
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
            scanner.fail(i18n::format(i18n::Message::ATTRIBUTE_VALUE_QUOTES));
        scanner.advance(1);
        std::string_view raw = scanner.readUntil(std::string_view(&quote, 1));
        if (!isNewAttributeName(attributes, name))
            scanner.failAt(i18n::format(i18n::Message::ATTRIBUTE_DUPLICATE, {name}), start);
        std::string value = AttributeValueDecoder(_dtd.get(), _budget, _standalone).decode(raw);
        attributes.push_back(Attribute{std::move(name), std::move(value)});
    }

    /*
    ** Нет ли уже атрибута с таким именем. Пока атрибутов мало — перебор без
    ** выделения памяти; на атрибуте номер utils::LINEAR_SEARCH_LIMIT имена переносятся в
    ** хеш-таблицу (она очищается заново для каждого большого тега), и
    ** дальше каждая проверка — O(1).
    **
    ** Whether there is already an attribute with this name. While there are few attributes — a loop without
    ** memory allocation; at the attribute number utils::LINEAR_SEARCH_LIMIT the names are moved into
    ** the hash table (it is cleared anew for every large tag), and
    ** from then on every check is O(1).
    */
    bool ParseSession::isNewAttributeName(const std::vector<Attribute>& attributes, const std::string& name)
    {
        if (attributes.size() < utils::LINEAR_SEARCH_LIMIT)
        {
            for (const Attribute& existing : attributes)
            {
                if (existing.name == name)
                    return false;
            }
            return true;
        }
        if (attributes.size() == utils::LINEAR_SEARCH_LIMIT)
        {
            _attributeNames.clear();
            for (const Attribute& existing : attributes)
                _attributeNames.insert(existing.name);
        }
        return _attributeNames.insert(name).second;
    }
}
