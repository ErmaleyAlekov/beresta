#include "xsd/internal/stream_identity.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Первый узел поля — атрибут: значение и канонический вид по типу
    ** (тип неизвестен — строка). Префиксы QName берутся у элемента-владельца.
    **
    ** The first node of a field is an attribute: the value and the canonical form by the type
    ** (the type is unknown — a string). QName prefixes are taken from the owner element.
    */
    static void assignAttribute(FieldState& field, std::string_view value, const SimpleType* type,
        const NamespaceLookup& namespaces)
    {
        if (type == nullptr)
            type = builtinType("string");
        field.fromElement = false;
        field.value.assign(value.data(), value.size());
        field.canonical = identityValue(*type, value, namespaces);
    }

    // Был ли атрибут с этим номером допущен шаблоном без проверки (нет типа)
    // Whether the attribute with this number was admitted by a wildcard without validation (no type)
    static bool isUntyped(const IdentityStart& start, std::size_t index) noexcept
    {
        for (std::size_t untyped : start.untypedAttributes)
        {
            if (untyped == index)
                return true;
        }
        return false;
    }

    // Атрибуты элемента, выбранные полем (объявления xmlns — не атрибуты, как в XPath)
    // The element attributes selected by a field (xmlns declarations are not attributes, as in XPath)
    void StreamIdentity::matchAttributes(FieldState& field, const std::vector<StreamPath>& paths,
        const ActiveTarget& target, const IdentityStart& start)
    {
        for (std::size_t index = 0; index < start.element.attributes.size(); ++index)
        {
            const SaxAttribute& attribute = start.element.attributes[index];
            bool reached = false;
            if (utils::isNamespaceDeclaration(attribute.name))
                continue;
            for (const StreamPath& path : paths)
                reached = reached || (path.attribute && nameMatches(path.attributeTest, attribute.namespaceUri,
                    attribute.localName) && pathReaches(path, _names, target.level, start.level));
            if (!reached || ++field.matches != 1)
                continue;
            const SimpleType* type = nullptr;
            if (index < start.attributeTypes.size())
                type = start.attributeTypes[index];
            assignAttribute(field, attribute.value, type, start.namespaces);
            field.untyped = isUntyped(start, index);
        }
    }

    /*
    ** Атрибуты со значением по умолчанию, которых нет в экземпляре: в PSVI
    ** они есть (3.4.4), поэтому поле выбирает их наравне с записанными
    ** (cvc-identity-constraint.3). Значение — default, иначе fixed.
    **
    ** Attributes with a default value that are absent in the instance: they are in
    ** the PSVI (3.4.4), so a field selects them just like the written ones
    ** (cvc-identity-constraint.3). The value is default, otherwise fixed.
    */
    void StreamIdentity::matchDefaults(FieldState& field, const std::vector<StreamPath>& paths,
        const ActiveTarget& target, const IdentityStart& start)
    {
        for (const AttributeUse* use : start.defaultAttributes)
        {
            bool reached = false;
            for (const StreamPath& path : paths)
                reached = reached || (path.attribute && nameMatches(path.attributeTest, use->namespaceUri,
                    use->name) && pathReaches(path, _names, target.level, start.level));
            if (!reached || ++field.matches != 1)
                continue;
            if (use->defaultValue.has_value())
                assignAttribute(field, use->defaultValue.value(), use->type, start.namespaces);
            else
                assignAttribute(field, use->fixedValue.value_or(std::string()), use->type, start.namespaces);
        }
    }
}
