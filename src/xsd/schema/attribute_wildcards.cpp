#include "xsd/internal/attribute_wildcards.hpp"

#include <algorithm>
#include <vector>

namespace beresta::xsd::detail
{
    using content::WildcardMode;

    /*
    ** У «не x» исключены и x, и атрибуты без пространства имён (2-я редакция,
    ** Wildcard allows Namespace Name, п. 2); у перечня — только его члены.
    **
    ** "Not x" excludes both x and attributes without a namespace (2nd edition,
    ** Wildcard allows Namespace Name, item 2); a list admits only its members.
    */
    bool wildcardAllows(const AttributeWildcard& wildcard, std::string_view namespaceUri) noexcept
    {
        if (wildcard.mode == WildcardMode::ANY_NAMESPACE)
            return true;
        if (wildcard.mode == WildcardMode::OTHER_NAMESPACE)
            return !namespaceUri.empty() && namespaceUri != wildcard.excluded;
        for (const std::string& allowed : wildcard.namespaces)
        {
            if (allowed == namespaceUri)
                return true;
        }
        return false;
    }

    // Одинаковы ли ограничения пространств имён (перечни — как множества)
    // Whether the namespace constraints are the same (lists — as sets)
    static bool sameConstraint(const AttributeWildcard& left, const AttributeWildcard& right)
    {
        if (left.mode != right.mode)
            return false;
        if (left.mode == WildcardMode::OTHER_NAMESPACE)
            return left.excluded == right.excluded;
        std::vector<std::string> a = left.namespaces;
        std::vector<std::string> b = right.namespaces;
        std::sort(a.begin(), a.end());
        std::sort(b.begin(), b.end());
        a.erase(std::unique(a.begin(), a.end()), a.end());
        b.erase(std::unique(b.begin(), b.end()), b.end());
        return a == b;
    }

    /*
    ** Пересечение: одинаковые — они же; ##any справа — левый; ##any или «не x»
    ** слева при перечне справа — то же с переставленными шаблонами; «не x» и
    ** «не y» — невыразимо (п. 5), но «не x» и «не пустого» — «не x» (п. 6);
    ** перечень слева — его члены, которые допускает правый шаблон (п. 3, 4).
    **
    ** The intersection: the same ones — themselves; ##any on the right — the left one; ##any or "not x"
    ** on the left with a list on the right — the same with the wildcards swapped; "not x" and
    ** "not y" — not expressible (item 5), but "not x" and "not empty" — "not x" (item 6);
    ** a list on the left — its members admitted by the right wildcard (items 3, 4).
    */
    std::optional<AttributeWildcard> intersectWildcards(const AttributeWildcard& left,
        const AttributeWildcard& right)
    {
        AttributeWildcard result = left;

        if (sameConstraint(left, right) || right.mode == WildcardMode::ANY_NAMESPACE)
            return result;
        bool otherWithList = left.mode == WildcardMode::OTHER_NAMESPACE
            && right.mode == WildcardMode::NAMESPACE_LIST;
        if (left.mode == WildcardMode::ANY_NAMESPACE || otherWithList)
        {
            std::optional<AttributeWildcard> swapped = intersectWildcards(right, left);
            if (swapped.has_value())
                swapped->process = left.process;
            return swapped;
        }
        if (left.mode == WildcardMode::OTHER_NAMESPACE)
        {
            if (!left.excluded.empty() && !right.excluded.empty())
                return std::nullopt;
            result.excluded = left.excluded + right.excluded;
            return result;
        }
        result.namespaces.clear();
        for (const std::string& uri : left.namespaces)
        {
            if (wildcardAllows(right, uri))
                result.namespaces.push_back(uri);
        }
        return result;
    }

    /*
    ** Объединение «не x» с перечнем S (п. 5, 6): x и «без пространства» в S —
    ** ##any; только x — «не пустого»; только «без пространства» — невыразимо;
    ** ни того ни другого — «не x». У «не пустого» (x пусто): «без пространства»
    ** в S — ##any, иначе — «не пустого».
    **
    ** The union of "not x" with a list S (items 5, 6): both x and "no namespace" in S —
    ** ##any; only x — "not empty"; only "no namespace" — not expressible;
    ** neither — "not x". For "not empty" (an empty x): "no namespace"
    ** in S — ##any, otherwise — "not empty".
    */
    static std::optional<AttributeWildcard> uniteWithNegation(const AttributeWildcard& negation,
        const AttributeWildcard& list, ProcessContents process)
    {
        AttributeWildcard result;
        bool hasAbsent = wildcardAllows(list, "");
        bool hasExcluded = !negation.excluded.empty() && wildcardAllows(list, negation.excluded);

        result.process = process;
        result.mode = WildcardMode::OTHER_NAMESPACE;
        if (hasAbsent && (hasExcluded || negation.excluded.empty()))
            result.mode = WildcardMode::ANY_NAMESPACE;
        else if (hasAbsent)
            return std::nullopt;
        else if (!hasExcluded)
            result.excluded = negation.excluded;
        return result;
    }

    /*
    ** Объединение: одинаковые — они же; есть ##any — ##any; два перечня —
    ** их объединение; «не x» и «не y» — «не пустого» (п. 4); остальное —
    ** «не x» с перечнем (uniteWithNegation).
    **
    ** The union: the same ones — themselves; ##any present — ##any; two lists —
    ** their union; "not x" and "not y" — "not empty" (item 4); the rest —
    ** "not x" with a list (uniteWithNegation).
    */
    std::optional<AttributeWildcard> uniteWildcards(const AttributeWildcard& left,
        const AttributeWildcard& right)
    {
        AttributeWildcard result = left;
        bool negations = left.mode == WildcardMode::OTHER_NAMESPACE && right.mode == WildcardMode::OTHER_NAMESPACE;

        if (negations)
            result.excluded.clear();
        if (sameConstraint(left, right))
            return left;
        if (negations)
            return result;
        if (left.mode == WildcardMode::ANY_NAMESPACE || right.mode == WildcardMode::ANY_NAMESPACE)
        {
            result.mode = WildcardMode::ANY_NAMESPACE;
            return result;
        }
        if (left.mode == WildcardMode::NAMESPACE_LIST && right.mode == WildcardMode::NAMESPACE_LIST)
        {
            for (const std::string& uri : right.namespaces)
            {
                if (!wildcardAllows(left, uri))
                    result.namespaces.push_back(uri);
            }
            return result;
        }
        if (left.mode == WildcardMode::OTHER_NAMESPACE)
            return uniteWithNegation(left, right, left.process);
        return uniteWithNegation(right, left, left.process);
    }
}
