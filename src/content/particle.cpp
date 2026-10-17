#include "content/particle.hpp"
#include "content/content_matcher.hpp"

namespace beresta::content
{
    /*
    ** Полная проверка: стартуем из позиции 0 и смотрим, достижим ли конец
    ** списка детей. При неудаче ошибка — на самой дальней позиции.
    */
    MatchResult matchContent(const Particle& model, const std::vector<ChildKey>& children)
    {
        ContentMatcher matcher(children);
        PositionSet starts(children.size() + 1, 0);

        starts[0] = 1;
        PositionSet ends = matcher.matchRepeated(model, starts);
        if (ends[children.size()])
            return MatchResult{true, 0};
        return MatchResult{false, matcher.getFurthest()};
    }

    // Пространство имён ребёнка входит в разрешённые для xs:any
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

    // Элемент: по полному имени (DTD) или по паре «пространство + локальное имя» (XSD)
    bool particleAccepts(const Particle& particle, const ChildKey& child) noexcept
    {
        if (particle.kind == ParticleKind::WILDCARD)
            return wildcardAccepts(particle, child.namespaceUri);
        if (particle.kind != ParticleKind::ELEMENT)
            return false;
        if (particle.byQualifiedName)
            return particle.name == child.qualifiedName;
        return particle.name == child.localName && particle.namespaceUri == child.namespaceUri;
    }

    // Суффикс повторений в нотации DTD: ?, *, + или {min,max}
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
    std::string describeParticle(const Particle& particle)
    {
        if (particle.kind == ParticleKind::ELEMENT)
            return particle.name + describeOccurrence(particle);
        if (particle.kind == ParticleKind::WILDCARD)
            return "*любой*" + describeOccurrence(particle);
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
