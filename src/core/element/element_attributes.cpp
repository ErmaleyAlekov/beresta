#include "core/element.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/xml_chars.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Установка атрибута: если атрибут уже есть — заменяем значение
    ** (позиция в списке сохраняется), иначе добавляем в конец.
    **
    ** Setting an attribute: if the attribute already exists, its value is replaced
    ** (the position in the list is kept), otherwise it is appended to the end.
    */
    void Element::setAttribute(std::string_view name, std::string value)
    {
        if (!utils::isValidName(name))
            throw DomException(DomErrorCode::INVALID_NAME,
                i18n::format(i18n::Message::ATTRIBUTE_NAME_INVALID, {name}));
        std::size_t index = findAttribute(name);
        if (index != npos)
        {
            _attributes[index].value = std::move(value);
            return;
        }
        _attributes.push_back(Attribute{std::string(name), std::move(value)});
    }

    // Возвращаем копию значения, чтобы не отдавать наружу ссылку на внутренности
    // A copy of the value is returned so as not to give out a reference to the internals
    std::optional<std::string> Element::getAttribute(std::string_view name) const
    {
        std::size_t index = findAttribute(name);

        if (index == npos)
            return std::nullopt;
        return _attributes[index].value;
    }

    // Проверка наличия атрибута
    // Checks whether an attribute exists
    bool Element::hasAttribute(std::string_view name) const noexcept
    {
        return findAttribute(name) != npos;
    }

    // Удаление с сохранением порядка остальных атрибутов
    // Removal keeping the order of the other attributes
    bool Element::removeAttribute(std::string_view name)
    {
        std::size_t index = findAttribute(name);

        if (index == npos)
            return false;
        _attributes.erase(_attributes.begin() + static_cast<std::ptrdiff_t>(index));
        return true;
    }

    // Все атрибуты только для чтения
    // All attributes, read-only
    const std::vector<Attribute>& Element::getAttributes() const noexcept
    {
        return _attributes;
    }
}
