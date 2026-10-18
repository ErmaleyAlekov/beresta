#include "core/element.hpp"
#include "core/text.hpp"

#include <utility>

namespace beresta
{
    // Резерв без изменения содержимого / A reservation without changing the content
    void Element::reserveAttributes(std::size_t count)
    {
        _attributes.reserve(count);
    }

    /*
    ** Поверхностная копия: имя и атрибуты переносятся, дети — нет.
    ** Создаём новый элемент с тем же именем и копируем вектор атрибутов
    ** напрямую (имена уже проверены, повторная проверка не нужна).
    **
    ** A shallow copy: the name and the attributes are carried over, the children are not.
    ** A new element with the same name is created and the attribute vector is copied
    ** directly (the names are already checked, a repeated check is not needed).
    */
    std::unique_ptr<Element> Element::cloneShallow() const
    {
        auto copy = std::make_unique<Element>(_name);

        copy->_attributes = _attributes;
        copy->setSourceLocation(getSourceLocation());
        return copy;
    }

    // Все дети заменяются одним текстовым узлом
    // All children are replaced with one text node
    void Element::setTextContent(std::string text)
    {
        clearChildren();
        if (text.empty())
            return;
        appendChild(std::make_unique<Text>(std::move(text)));
    }

    // Элементы равны, если совпадают тип, имя, атрибуты и все дети попарно
    // Elements are equal if the type, the name, the attributes and all children match pairwise
    bool Element::isEqualNode(const Node& other) const
    {
        if (other.getType() != NodeType::ELEMENT)
            return false;
        const Element& element = static_cast<const Element&>(other);
        if (!equalsShallow(element))
            return false;
        return childrenEqual(element);
    }
}
