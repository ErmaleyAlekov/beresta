#include "xsd/internal/prefix_bindings.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    // Пустой стек: действуют только встроенные префиксы xml и xmlns
    // An empty stack: only the built-in xml and xmlns prefixes are in effect
    PrefixBindings::PrefixBindings()
        : _bindings(), _used(0), _marks()
    {
    }

    /*
    ** Отметка начала элемента и копии его объявлений (строки переиспользуют
    ** память). Повтор префикса в одном элементе (возможен лишь в дереве,
    ** собранном вручную) не перекрывает первое объявление — как у поиска
    ** Element::lookupNamespaceURI, который берёт первый подходящий атрибут.
    **
    ** The mark of the element start and copies of its declarations (the strings reuse
    ** memory). A repeated prefix in one element (possible only in a tree
    ** assembled by hand) does not override the first declaration — like the
    ** Element::lookupNamespaceURI lookup, which takes the first matching attribute.
    */
    bool PrefixBindings::push(const std::vector<SaxAttribute>& attributes)
    {
        std::size_t mark = _used;
        bool declares = false;

        _marks.push_back(mark);
        for (const SaxAttribute& attribute : attributes)
        {
            // Первая буква отсекает почти все атрибуты без сравнения строк
            // The first letter cuts off almost all attributes without comparing strings
            if (attribute.name.empty() || attribute.name[0] != 'x' || !utils::isNamespaceDeclaration(attribute.name))
                continue;
            declares = true;
            std::string_view prefix = utils::declaredPrefix(attribute.name);
            bool repeated = false;
            for (std::size_t index = mark; index < _used; ++index)
                repeated = repeated || _bindings[index].prefix == prefix;
            if (repeated)
                continue;
            if (_used == _bindings.size())
                _bindings.emplace_back();
            _bindings[_used].prefix.assign(prefix.data(), prefix.size());
            _bindings[_used].namespaceUri.assign(attribute.value.data(), attribute.value.size());
            ++_used;
        }
        return declares;
    }

    // Объявления закрытого элемента снимаются разом — до его отметки
    // The declarations of the closed element are removed at once — down to its mark
    void PrefixBindings::pop()
    {
        if (_marks.empty())
            return;
        _used = _marks.back();
        _marks.pop_back();
    }

    // Ближайшее объявление побеждает; пустой URI у префикса — отмена (как Element::lookupNamespaceURI)
    // The nearest declaration wins; an empty URI of a prefix is a cancellation (like Element::lookupNamespaceURI)
    std::optional<std::string_view> PrefixBindings::lookup(std::string_view prefix) const
    {
        if (prefix == "xml")
            return XML_NAMESPACE_URI;
        if (prefix == "xmlns")
            return XMLNS_NAMESPACE_URI;
        for (std::size_t index = _used; index > 0; --index)
        {
            const PrefixBinding& binding = _bindings[index - 1];
            if (binding.prefix != prefix)
                continue;
            if (binding.namespaceUri.empty())
                return std::nullopt;
            return std::string_view(binding.namespaceUri);
        }
        return std::nullopt;
    }

    // Префикс объявлен, если поиск нашёл непустой URI
    // A prefix is declared if the lookup found a non-empty URI
    bool PrefixBindings::isBound(std::string_view prefix) const
    {
        return lookup(prefix).has_value();
    }
}
