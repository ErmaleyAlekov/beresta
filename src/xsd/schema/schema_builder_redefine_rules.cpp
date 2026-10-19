#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Ссылки компонента xs:redefine на прежнее определение: потомки того
    ** же вида (xs:group внутри группы, xs:attributeGroup внутри группы
    ** атрибутов), чей ref разрешается в его же имя. Обход без рекурсии —
    ** явным стеком; аннотации и чужие пространства имён не смотрятся.
    **
    ** The references of an xs:redefine component to the former definition: descendants of the
    ** same kind (xs:group inside a group, xs:attributeGroup inside an attribute
    ** group) whose ref resolves to its own name. The traversal is without recursion —
    ** with an explicit stack; annotations and foreign namespaces are not looked at.
    */
    std::vector<const Element*> SchemaBuilder::selfReferences(const Element& component,
        const QualifiedKey& key) const
    {
        std::string_view kind = component.getLocalName();
        std::vector<const Element*> pending = schemaChildren(component);
        std::vector<const Element*> found;

        while (!pending.empty())
        {
            const Element* element = pending.back();
            pending.pop_back();
            std::optional<std::string> reference = element->getAttribute("ref");
            if (element->getLocalName() == kind && reference.has_value()
                && resolveQName(*element, reference.value()) == key)
                found.push_back(element);
            for (const Element* child : schemaChildren(*element))
                pending.push_back(child);
        }
        return found;
    }

    /*
    ** src-redefine.6.1 и 7.1: группа (группа атрибутов) внутри xs:redefine
    ** ссылается на прежнее определение не больше одного раза; у такой
    ** ссылки группы minOccurs и maxOccurs равны 1 — иначе «расширение»
    ** повторяло бы прежнюю модель. Без ссылки на себя новая группа должна
    ** быть корректным ограничением прежней (6.2, 7.2) — это не проверяется,
    ** как и cos-particle-restrict (см. «Ограничения» в docs/comparison.md).
    **
    ** src-redefine.6.1 and 7.1: a group (an attribute group) inside xs:redefine
    ** refers to the former definition at most once; such a reference
    ** of a group has minOccurs and maxOccurs equal to 1 — otherwise the "extension"
    ** would repeat the former model. Without a self-reference the new group must
    ** be a valid restriction of the former one (6.2, 7.2) — this is not checked,
    ** just like cos-particle-restrict (see "Limitations" in docs/comparison.md).
    */
    void SchemaBuilder::checkSelfReferences(const Element& child, const QualifiedKey& key) const
    {
        bool group = isSchemaElement(child, "group");

        if (!group && !isSchemaElement(child, "attributeGroup"))
            return;
        std::vector<const Element*> references = selfReferences(child, key);
        if (references.size() > 1)
            fail(*references[1], i18n::format(i18n::Message::SCHEMA_REDEFINE_SELF_REFERENCES, {key.second}));
        if (references.empty() || !group)
            return;
        bool once = attributeOr(*references.front(), "minOccurs", "1") == "1"
            && attributeOr(*references.front(), "maxOccurs", "1") == "1";
        if (!once)
            fail(*references.front(), i18n::format(i18n::Message::SCHEMA_REDEFINE_SELF_OCCURS, {key.second}));
    }

    /*
    ** Загружен ли документ uri (транзитивно) изнутри документа ancestor:
    ** подъём по цепочке «кто загрузил». Цепочка не длиннее числа
    ** загруженных документов — это защита от цикла.
    **
    ** Whether the document uri was loaded (transitively) from inside the document ancestor:
    ** climbing the "who loaded it" chain. The chain is not longer than the number
    ** of loaded documents — this is a protection against a cycle.
    */
    bool SchemaBuilder::loadedWithin(const std::string& uri, const std::string& ancestor) const
    {
        std::string current = uri;

        for (std::size_t steps = 0; steps <= _loadedBy.size(); ++steps)
        {
            auto loader = _loadedBy.find(current);
            if (loader == _loadedBy.end())
                return false;
            if (loader->second == ancestor)
                return true;
            current = loader->second;
        }
        return false;
    }

    /*
    ** Именованные группы и группы атрибутов подставляются в типы при
    ** ссылке, поэтому неиспользуемая группа раньше не строилась вовсе и
    ** ошибки в ней (неизвестный тип, ссылка на несуществующую группу)
    ** проходили незамеченными. Здесь каждая строится один раз во
    ** временный тип — результат отбрасывается, важны только ошибки.
    ** Локальные элементы групп при этом регистрируются и затем
    ** переиспользуются (они строятся один раз на узел XML).
    **
    ** Named groups and attribute groups are substituted into types on
    ** reference, so an unused group was previously not built at all and
    ** errors in it (an unknown type, a reference to a non-existent group)
    ** went unnoticed. Here each one is built once into a temporary type — the result
    ** is discarded, only the errors matter. The local elements of the groups
    ** are registered and later reused (they are built once per XML node).
    */
    void SchemaBuilder::buildNamedGroups()
    {
        for (const auto& entry : _definitions["group"])
        {
            std::vector<const Element*> model = schemaChildren(*entry.second.element);
            ComplexType scratch;
            checkNamedGroup(*entry.second.element);
            if (!model.empty())
                buildParticle(*model.front(), *entry.second.source, scratch, 1);
        }
        for (const auto& entry : _definitions["attributeGroup"])
        {
            ComplexType scratch;
            buildAttributes(*entry.second.element, *entry.second.source, scratch, 1);
        }
    }

    /*
    ** Именованная группа (глобальная или внутри xs:redefine) — схема для
    ** схем, namedGroup: у самой группы нет ref, minOccurs и maxOccurs, у
    ** её модели (xs:all / xs:choice / xs:sequence) — minOccurs и maxOccurs.
    ** Повторения задаёт только место ссылки на группу.
    **
    ** A named group (a global one or inside xs:redefine) — the schema for
    ** schemas, namedGroup: the group itself has no ref, minOccurs and maxOccurs, its
    ** model (xs:all / xs:choice / xs:sequence) has no minOccurs and maxOccurs.
    ** Only the place of the group reference sets the repetitions.
    */
    void SchemaBuilder::checkNamedGroup(const Element& group) const
    {
        static const std::string_view GROUP_FORBIDDEN[] = {"ref", "minOccurs", "maxOccurs"};
        std::string name = attributeOr(group, "name", "");
        std::vector<const Element*> model = schemaChildren(group);

        for (std::string_view attribute : GROUP_FORBIDDEN)
        {
            bool onModel = !model.empty() && attribute != "ref" && model.front()->hasAttribute(attribute);
            if (group.hasAttribute(attribute) || onModel)
                fail(group, i18n::format(i18n::Message::SCHEMA_NAMED_GROUP_FORBIDDEN, {name, attribute}));
        }
    }
}
