#include "content/particle.hpp"

namespace beresta::content
{
    /*
    ** Сравнение имени из группы с именем ребёнка: сначала пространство
    ** имён, затем локальное имя (тот же порядок, что у operator<).
    ** Отрицательное — имя группы меньше, ноль — совпадение.
    **
    ** Comparing a name from the group with the child's name: the namespace
    ** first, then the local name (the same order as operator<).
    ** Negative — the group name is smaller, zero — a match.
    */
    static int compareName(const ElementName& name, const ChildKey& child) noexcept
    {
        int byNamespace = std::string_view(name.namespaceUri).compare(child.namespaceUri);

        if (byNamespace != 0)
            return byNamespace;
        return std::string_view(name.localName).compare(child.localName);
    }

    // Пара (пространство имён, локальное имя) — ключ упорядочивания группы
    // The pair (namespace, local name) is the ordering key of the group
    bool operator<(const ElementName& left, const ElementName& right) noexcept
    {
        if (left.namespaceUri != right.namespaceUri)
            return left.namespaceUri < right.namespaceUri;
        return left.localName < right.localName;
    }

    /*
    ** Двоичный поиск по упорядоченной группе: O(log n) сравнений строк.
    ** Вызывается только тогда, когда имя ребёнка не совпало с самой
    ** частицей, поэтому модели без групп замены его не оплачивают.
    **
    ** A binary search over the ordered group: O(log n) string comparisons.
    ** It is called only when the child's name did not match the particle
    ** itself, so models without substitution groups do not pay for it.
    */
    bool isSubstitute(const SubstitutionGroup& group, const ChildKey& child) noexcept
    {
        std::size_t low = 0;
        std::size_t high = group.size();

        while (low < high)
        {
            std::size_t middle = low + (high - low) / 2;
            int order = compareName(group[middle], child);
            if (order == 0)
                return true;
            if (order < 0)
                low = middle + 1;
            else
                high = middle;
        }
        return false;
    }

    // Флаг частицы: указатель на группу есть, и в группе есть хоть один член
    // The particle flag: there is a pointer to a group, and the group has at least one member
    bool hasSubstitutes(const Particle& particle) noexcept
    {
        return particle.substitutes != nullptr && !particle.substitutes->empty();
    }
}
