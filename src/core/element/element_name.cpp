#include "core/element.hpp"
#include "exception/dom_exception.hpp"
#include "utils/xml_chars.hpp"
#include "utils/xml_escape.hpp"

#include <utility>

namespace beresta
{
    // Имя тега
    const std::string& Element::getName() const noexcept
    {
        return _name;
    }

    // Новое имя принимается только если оно является корректным XML-именем
    void Element::setName(std::string name)
    {
        if (!utils::isValidName(name))
            throw DomException(DomErrorCode::INVALID_NAME,
                "недопустимое имя элемента: \"" + name + "\"");
        _name = std::move(name);
    }

    // Линейный поиск атрибута по имени (атрибутов обычно единицы)
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
