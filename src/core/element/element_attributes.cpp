#include "core/element.hpp"
#include "exception/dom_exception.hpp"
#include "utils/xml_chars.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Установка атрибута: если атрибут уже есть — заменяем значение
    ** (позиция в списке сохраняется), иначе добавляем в конец.
    */
    void Element::setAttribute(std::string_view name, std::string value)
    {
        if (!utils::isValidName(name))
            throw DomException(DomErrorCode::INVALID_NAME,
                "недопустимое имя атрибута: \"" + std::string(name) + "\"");
        std::size_t index = findAttribute(name);
        if (index != npos)
        {
            _attributes[index].value = std::move(value);
            return;
        }
        _attributes.push_back(Attribute{std::string(name), std::move(value)});
    }

    // Возвращаем копию значения, чтобы не отдавать наружу ссылку на внутренности
    std::optional<std::string> Element::getAttribute(std::string_view name) const
    {
        std::size_t index = findAttribute(name);

        if (index == npos)
            return std::nullopt;
        return _attributes[index].value;
    }

    // Проверка наличия атрибута
    bool Element::hasAttribute(std::string_view name) const noexcept
    {
        return findAttribute(name) != npos;
    }

    // Удаление с сохранением порядка остальных атрибутов
    bool Element::removeAttribute(std::string_view name)
    {
        std::size_t index = findAttribute(name);

        if (index == npos)
            return false;
        _attributes.erase(_attributes.begin() + static_cast<std::ptrdiff_t>(index));
        return true;
    }

    // Все атрибуты только для чтения
    const std::vector<Attribute>& Element::getAttributes() const noexcept
    {
        return _attributes;
    }
}
