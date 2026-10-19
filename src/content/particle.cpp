#include "content/particle.hpp"
#include "i18n/i18n.hpp"
#include "content/content_matcher.hpp"

namespace beresta::content
{
    /*
    ** Полная проверка: стартуем из позиции 0 и смотрим, достижим ли конец
    ** списка детей. При неудаче ошибка — на самой дальней позиции.
    **
    ** The full check: start from position 0 and see whether the end of the
    ** child list is reachable. On failure the error is at the farthest position.
    */
    MatchResult matchContent(const Particle& model, const std::vector<ChildKey>& children)
    {
        // Частый случай проверяется одним проходом; общий алгоритм — для остального и для ошибок
        // The common case is checked in one pass; the general algorithm is for the rest and for errors
        if (matchesSimpleSequence(model, children))
            return MatchResult{true, 0};
        ContentMatcher matcher(children);
        PositionSet ends = matcher.matchRepeated(model, PositionSet{0});

        if (!ends.empty() && ends.back() == children.size())
            return MatchResult{true, 0};
        return MatchResult{false, matcher.getFurthest()};
    }

    // Пространство имён ребёнка входит в разрешённые для xs:any
    // The child's namespace is among the ones allowed for xs:any
    static bool wildcardAccepts(const Particle& particle, std::string_view namespaceUri) noexcept
    {
        if (particle.wildcardMode == WildcardMode::ANY_NAMESPACE)
            return true;
        if (particle.wildcardMode == WildcardMode::OTHER_NAMESPACE)
            return !namespaceUri.empty() && namespaceUri != particle.namespaceUri;
        for (const std::string& allowed : particle.wildcardNamespaces)
        {
            if (allowed == namespaceUri)
                return true;
        }
        return false;
    }

    /*
    ** Элемент: по полному имени (DTD) или по паре «пространство + локальное
    ** имя» (XSD). Не совпавшее имя ещё может принадлежать члену группы
    ** замены — эта проверка идёт последней и только при наличии группы.
    **
    ** An element: by the qualified name (DTD) or by the pair "namespace + local
    ** name" (XSD). A name that did not match may still belong to a member of the
    ** substitution group — this check goes last and only when there is a group.
    */
    bool particleAccepts(const Particle& particle, const ChildKey& child) noexcept
    {
        if (particle.kind == ParticleKind::WILDCARD)
            return wildcardAccepts(particle, child.namespaceUri);
        if (particle.kind != ParticleKind::ELEMENT)
            return false;
        if (particle.byQualifiedName)
            return particle.name == child.qualifiedName;
        if (particle.name == child.localName && particle.namespaceUri == child.namespaceUri)
            return true;
        return particle.substitutes != nullptr && isSubstitute(*particle.substitutes, child);
    }

    // Суффикс повторений в нотации DTD: ?, *, + или {min,max}
    // The repetition suffix in DTD notation: ?, *, + or {min,max}
    static std::string describeOccurrence(const Particle& particle)
    {
        if (particle.minOccurs == 1 && particle.maxOccurs == 1)
            return "";
        if (particle.minOccurs == 0 && particle.maxOccurs == 1)
            return "?";
        if (particle.minOccurs == 0 && particle.maxOccurs == UNBOUNDED)
            return "*";
        if (particle.minOccurs == 1 && particle.maxOccurs == UNBOUNDED)
            return "+";
        std::string text = "{" + std::to_string(particle.minOccurs) + ",";
        if (particle.maxOccurs != UNBOUNDED)
            text += std::to_string(particle.maxOccurs);
        return text + "}";
    }

    // Рекурсивная запись модели: группы в скобках с разделителями ", " / " | " / " & "
    // A recursive notation of the model: groups in parentheses with the separators ", " / " | " / " & "
    std::string describeParticle(const Particle& particle)
    {
        if (particle.kind == ParticleKind::ELEMENT)
            return particle.name + describeOccurrence(particle);
        if (particle.kind == ParticleKind::WILDCARD)
            return i18n::format(i18n::Message::MODEL_ANY) + describeOccurrence(particle);
        std::string separator = ", ";
        if (particle.kind == ParticleKind::CHOICE)
            separator = " | ";
        if (particle.kind == ParticleKind::ALL)
            separator = " & ";
        std::string text = "(";
        for (std::size_t index = 0; index < particle.children.size(); ++index)
        {
            if (index > 0)
                text += separator;
            text += describeParticle(particle.children[index]);
        }
        return text + ")" + describeOccurrence(particle);
    }
}
