#include "xsd/internal/prefix_bindings.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Все действующие объявления: стек просматривается с вершины, и каждый
    ** префикс берётся из ближайшего объявления (дальние им перекрыты).
    ** Нужно утверждениям: корень мини-дерева получает префиксы предков.
    **
    ** All the declarations in effect: the stack is scanned from the top, and each
    ** prefix is taken from the nearest declaration (the farther ones are hidden by it).
    ** Assertions need it: the root of a mini-tree gets the prefixes of the ancestors.
    */
    std::vector<PrefixBinding> PrefixBindings::inScope() const
    {
        std::vector<PrefixBinding> result;

        for (std::size_t index = _used; index > 0; --index)
        {
            const PrefixBinding& binding = _bindings[index - 1];
            bool hidden = false;
            for (const PrefixBinding& seen : result)
                hidden = hidden || seen.prefix == binding.prefix;
            if (!hidden)
                result.push_back(binding);
        }
        return result;
    }

    // Тот же поиск, что у lookup, но строка копируется: ключ переживёт закрытие элемента
    // The same lookup as in lookup, but the string is copied: the key outlives closing the element
    std::optional<std::string> PrefixBindings::namespaceOf(std::string_view prefix) const
    {
        std::optional<std::string_view> found = lookup(prefix);

        if (!found.has_value())
            return std::nullopt;
        return std::string(found.value());
    }
}
