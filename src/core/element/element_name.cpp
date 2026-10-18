#include "core/element.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/xml_chars.hpp"
#include "utils/xml_escape.hpp"

#include <utility>

namespace beresta
{
    // Имя тега
    // The tag name
    const std::string& Element::getName() const noexcept
    {
        return _name;
    }

    // Новое имя принимается только если оно является корректным XML-именем
    // A new name is accepted only if it is a valid XML name
    void Element::setName(std::string name)
    {
        if (!utils::isValidName(name))
            throw DomException(DomErrorCode::INVALID_NAME,
                i18n::format(i18n::Message::ELEMENT_NAME_INVALID, {name}));
        _name = std::move(name);
    }

    // Линейный поиск атрибута по имени (атрибутов обычно единицы)
    // A linear search for an attribute by name (there are usually only a few attributes)
    std::size_t Element::findAttribute(std::string_view name) const noexcept
    {
        for (std::size_t index = 0; index < _attributes.size(); ++index)
        {
            if (_attributes[index].name == name)
                return index;
        }
        return npos;
    }

    // Открывающий тег без закрывающей скобки: её выбирает вызывающий ('>' или "/>")
    // The start tag without the closing bracket: the caller chooses it ('>' or "/>")
    void Element::serializeStartTag(std::string& output) const
    {
        output += '<';
        output += _name;
        serializeAttributes(output);
    }

    /*
    ** Каждый атрибут выводится как ` name="value"`: ведущий пробел —
    ** обязательный разделитель синтаксиса, других пробелов нет.
    ** Значение всегда в двойных кавычках, внутренние кавычки экранируются.
    **
    ** Each attribute is written as ` name="value"`: the leading space is
    ** the mandatory syntax separator, there are no other spaces.
    ** The value is always in double quotes, inner quotes are escaped.
    */
    void Element::serializeAttributes(std::string& output) const
    {
        for (const Attribute& attribute : _attributes)
        {
            output += ' ';
            output += attribute.name;
            output += "=\"";
            utils::appendEscapedAttribute(output, attribute.value);
            output += '"';
        }
    }
}
