#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** Частица модели: элемент, xs:any, ссылка на группу или вложенная
    ** группа sequence / choice / all. Глубина вложенности ограничена.
    **
    ** A model particle: an element, xs:any, a group reference or a nested
    ** sequence / choice / all group. The nesting depth is limited.
    */
    content::Particle SchemaBuilder::buildParticle(const Element& definition, const SchemaSource& source,
        ComplexType& owner, std::size_t depth)
    {
        std::string_view kind = definition.getLocalName();
        content::Particle group;

        if (depth > 64)
            fail(definition, i18n::format(i18n::Message::MODEL_TOO_DEEP));
        if (kind == "element")
            return buildElementParticle(definition, source, owner);
        if (kind == "any")
            return buildWildcard(definition, source, owner);
        if (kind == "group")
            return buildGroupReference(definition, owner, depth);
        if (kind != "sequence" && kind != "choice" && kind != "all")
            fail(definition, i18n::format(i18n::Message::SCHEMA_UNEXPECTED_IN_MODEL, {kind}));
        if (kind == "choice")
            group.kind = content::ParticleKind::CHOICE;
        if (kind == "all")
            group.kind = content::ParticleKind::ALL;
        for (const Element* child : schemaChildren(definition))
        {
            // Частица с maxOccurs="0" не порождает компонента (3.9.2), но ошибки в ней проверены.
            // В xs:all любой ребёнок допускается один раз, поэтому такую частицу надо убрать;
            // в sequence / choice её и так нельзя повторить ни разу.
            // A particle with maxOccurs="0" yields no component (3.9.2), but its errors are checked.
            // In xs:all any child is allowed once, so such a particle must be removed;
            // in sequence / choice it cannot be repeated even once anyway.
            content::Particle part = buildParticle(*child, source, owner, depth + 1);
            if (part.maxOccurs != 0 || group.kind != content::ParticleKind::ALL)
                group.children.push_back(std::move(part));
        }
        readOccurs(definition, group);
        return group;
    }

    // <xs:group ref="g"/>: модель именованной группы с повторениями из места ссылки
    // <xs:group ref="g"/>: the model of a named group with the repetitions from the reference place
    content::Particle SchemaBuilder::buildGroupReference(const Element& reference, ComplexType& owner,
        std::size_t depth)
    {
        std::string name = attributeOr(reference, "ref", "");
        const Definition* named = referencedDefinition(reference, "group", resolveQName(reference, name));
        std::vector<const Element*> model;

        if (named != nullptr)
            model = schemaChildren(*named->element);
        if (model.empty())
            fail(reference, i18n::format(i18n::Message::SCHEMA_GROUP_MISSING, {name}));
        content::Particle group = buildParticle(*model.front(), *named->source, owner, depth + 1);
        readOccurs(reference, group);
        return group;
    }

    /*
    ** Элемент модели: локальное объявление или ссылка ref на глобальное.
    ** Ссылка получает указатель на группу замены глобального элемента:
    ** сама группа ещё пуста и будет заполнена после загрузки схемы.
    ** Локальное объявление строится один раз на узел XML (повторная
    ** подстановка именованной группы берёт готовое) — проверка его
    ** атрибутов тоже выполняется один раз.
    **
    ** A model element: a local declaration or a ref reference to a global one.
    ** A reference gets a pointer to the substitution group of the global element:
    ** the group itself is still empty and will be filled after the schema is loaded.
    ** A local declaration is built once per XML node (a repeated
    ** substitution of a named group takes the ready one) — the check of its
    ** attributes is also done once.
    */
    content::Particle SchemaBuilder::buildElementParticle(const Element& definition,
        const SchemaSource& source, ComplexType& owner)
    {
        const ElementDecl* declaration = nullptr;
        std::optional<std::string> reference = definition.getAttribute("ref");
        content::Particle particle;

        if (reference.has_value())
        {
            checkElementAttributes(definition, ElementPlace::ELEMENT_REFERENCE);
            declaration = globalElement(resolveQName(definition, reference.value()));
            particle.substitutes = &declaration->substitutes;
        }
        else if (_builtLocals.count(&definition) != 0)
            declaration = _builtLocals[&definition];
        else
        {
            checkElementAttributes(definition, ElementPlace::LOCAL_ELEMENT);
            declaration = buildElement(definition, source, false);
        }
        particle.kind = content::ParticleKind::ELEMENT;
        particle.name = declaration->name;
        particle.namespaceUri = declaration->namespaceUri;
        readOccurs(definition, particle);
        owner.elements.push_back(declaration);
        return particle;
    }

    /*
    ** xs:any: ограничение пространств имён (##any, ##other, ##local,
    ** ##targetNamespace или список URI) и режим processContents.
    **
    ** xs:any: the namespace constraint (##any, ##other, ##local,
    ** ##targetNamespace or a list of URIs) and the processContents mode.
    */
    content::Particle SchemaBuilder::buildWildcard(const Element& definition, const SchemaSource& source,
        ComplexType& owner)
    {
        content::Particle particle;

        particle.kind = content::ParticleKind::WILDCARD;
        particle.namespaceUri = source.targetNamespace;
        readNamespaceConstraint(definition, source.targetNamespace, particle.wildcardMode,
            particle.wildcardNamespaces);
        owner.hasElementWildcard = true;
        owner.elementWildcard = parseProcessContents(definition);
        readOccurs(definition, particle);
        return particle;
    }
}
