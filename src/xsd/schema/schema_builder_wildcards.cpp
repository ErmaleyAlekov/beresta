#include "xsd/internal/schema_builder.hpp"
#include "xsd/internal/attribute_wildcards.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_tokens.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** ##any — любое пространство, ##other — «не целевое и не пустое»
    ** (у шаблона элемента исключаемое пространство — его namespaceUri,
    ** поэтому список здесь не заполняется), остальное — перечень URI.
    **
    ** ##any — any namespace, ##other — "not the target one and not empty"
    ** (for an element wildcard the excluded namespace is its namespaceUri,
    ** so the list is not filled here), the rest — a list of URIs.
    */
    void readNamespaceConstraint(const Element& wildcard, const std::string& targetNamespace,
        content::WildcardMode& mode, std::vector<std::string>& namespaces)
    {
        std::string text = attributeOr(wildcard, "namespace", "##any");

        mode = content::WildcardMode::NAMESPACE_LIST;
        if (text == "##any")
            mode = content::WildcardMode::ANY_NAMESPACE;
        if (text == "##other")
            mode = content::WildcardMode::OTHER_NAMESPACE;
        if (mode != content::WildcardMode::NAMESPACE_LIST)
            return;
        for (std::string_view token : utils::splitTokens(text))
        {
            std::string uri(token);
            if (token == "##targetNamespace")
                uri = targetNamespace;
            if (token == "##local")
                uri.clear();
            namespaces.push_back(uri);
        }
    }

    // Собственный xs:anyAttribute: ограничение пространств имён и processContents
    // The own xs:anyAttribute: the namespace constraint and processContents
    AttributeWildcard SchemaBuilder::readAttributeWildcard(const Element& definition,
        const SchemaSource& source) const
    {
        AttributeWildcard wildcard;

        readNamespaceConstraint(definition, source.targetNamespace, wildcard.mode, wildcard.namespaces);
        if (wildcard.mode == content::WildcardMode::OTHER_NAMESPACE)
            wildcard.excluded = source.targetNamespace;
        wildcard.process = parseProcessContents(definition);
        return wildcard;
    }

    /*
    ** Пересекает накопленный шаблон со следующим (группы атрибутов, затем
    ** собственный xs:anyAttribute). processContents остаётся у накопленного:
    ** у первой группы, а при собственном шаблоне — у него (3.4.2, п. 2.1.2–2.1.3).
    **
    ** Intersects the accumulated wildcard with the next one (attribute groups, then
    ** the own xs:anyAttribute). processContents stays with the accumulated one:
    ** of the first group, and with an own wildcard — of it (3.4.2, items 2.1.2–2.1.3).
    */
    void SchemaBuilder::mergeGroupWildcard(std::optional<AttributeWildcard>& total,
        const std::optional<AttributeWildcard>& next, const Element& where) const
    {
        if (!next.has_value())
            return;
        if (!total.has_value())
        {
            total = next;
            return;
        }
        total = intersectWildcards(*total, *next);
        if (!total.has_value())
            fail(where, i18n::format(i18n::Message::SCHEMA_WILDCARD_INEXPRESSIBLE));
    }

    // Локальный «полный» шаблон становится шаблоном типа (при ограничении — окончательно)
    // The local "complete" wildcard becomes the type's wildcard (on restriction — finally)
    void SchemaBuilder::applyAttributeWildcard(ComplexType& type, std::optional<AttributeWildcard> local) const
    {
        type.anyAttribute = local.has_value();
        if (local.has_value())
            type.attributeWildcard = std::move(*local);
    }

    /*
    ** Расширение: шаблон базового типа объединяется с собственным
    ** (processContents — собственного), а без собственного переходит как есть.
    ** При ограничении шаблон базового типа не наследуется (3.4.2, п. 2.2).
    **
    ** Extension: the base type's wildcard is united with the own one
    ** (processContents — of the own one), and without an own one it passes as is.
    ** On restriction the base type's wildcard is not inherited (3.4.2, item 2.2).
    */
    void SchemaBuilder::inheritAttributeWildcard(ComplexType& type, const ComplexType& base,
        const Element& where) const
    {
        if (!base.anyAttribute)
            return;
        if (!type.anyAttribute)
        {
            type.anyAttribute = true;
            type.attributeWildcard = base.attributeWildcard;
            return;
        }
        std::optional<AttributeWildcard> united = uniteWildcards(type.attributeWildcard, base.attributeWildcard);
        if (!united.has_value())
            fail(where, i18n::format(i18n::Message::SCHEMA_WILDCARD_INEXPRESSIBLE));
        type.attributeWildcard = std::move(*united);
    }
}
