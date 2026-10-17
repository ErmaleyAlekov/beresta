#include "core/element.hpp"
#include "core/text.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Поверхностная копия: имя и атрибуты переносятся, дети — нет.
    ** Создаём новый элемент с тем же именем и копируем вектор атрибутов
    ** напрямую (имена уже проверены, повторная проверка не нужна).
    */
    std::unique_ptr<Element> Element::cloneShallow() const
    {
        auto copy = std::make_unique<Element>(_name);

        copy->_attributes = _attributes;
        copy->setSourceLocation(getSourceLocation());
        return copy;
    }

    // Все дети заменяются одним текстовым узлом
    void Element::setTextContent(std::string text)
    {
        clearChildren();
        if (text.empty())
            return;
        appendChild(std::make_unique<Text>(std::move(text)));
    }

    /*
    ** Равенство атрибутов как множеств: порядок не важен (так требует DOM),
    ** важны совпадение количества и значение каждого атрибута по имени.
    */
    static bool attributesEqual(const std::vector<Attribute>& left, const Element& right)
    {
        if (left.size() != right.getAttributes().size())
            return false;
        for (const Attribute& attribute : left)
        {
            std::optional<std::string> value = right.getAttribute(attribute.name);
            if (!value.has_value() || value.value() != attribute.value)
                return false;
        }
        return true;
    }

    // Элементы равны, если совпадают тип, имя, атрибуты и все дети попарно
    bool Element::isEqualNode(const Node& other) const
    {
        if (other.getType() != NodeType::ELEMENT)
            return false;
        const Element& element = static_cast<const Element&>(other);
        if (element._name != _name || !attributesEqual(_attributes, element))
            return false;
        return childrenEqual(element);
    }
}
