#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Проверка формы xs:attribute (схема для схем и src-attribute):
    **  - у глобального объявления нет ref, form и use (topLevelAttribute);
    **  - use — только optional, required или prohibited;
    **  - вместе с default атрибут use может быть только optional
    **    (src-attribute.2). Правило касается именно default: fixed вместе с
    **    use="required" или use="prohibited" допустим (attKb008, attKb009).
    ** Проверяются атрибуты самого элемента: default глобального объявления,
    ** на которое ссылается ref, к use этой ссылки отношения не имеет.
    **
    ** Checking the form of xs:attribute (the schema for schemas and src-attribute):
    **  - a global declaration has no ref, form and use (topLevelAttribute);
    **  - use is only optional, required or prohibited;
    **  - together with default the use attribute can only be optional
    **    (src-attribute.2). The rule is about default only: fixed together with
    **    use="required" or use="prohibited" is allowed (attKb008, attKb009).
    ** The attributes of the element itself are checked: the default of the global
    ** declaration a ref refers to has nothing to do with the use of that reference.
    */
    void SchemaBuilder::checkAttributeForm(const Element& definition, const SchemaSource& source, bool global) const
    {
        static const std::string_view GLOBAL_FORBIDDEN[] = {"ref", "form", "use"};
        std::string name = attributeOr(definition, "name", attributeOr(definition, "ref", ""));
        std::string raw = attributeOr(definition, "use", "optional");
        std::string usage = normalizeWhiteSpace(raw, WhiteSpace::COLLAPSE);

        for (std::string_view attribute : GLOBAL_FORBIDDEN)
        {
            if (global && definition.hasAttribute(attribute))
                fail(definition, i18n::format(i18n::Message::SCHEMA_GLOBAL_ATTRIBUTE_FORBIDDEN, {name, attribute}));
        }
        if (usage != "optional" && usage != "required" && usage != "prohibited")
            fail(definition, i18n::format(i18n::Message::SCHEMA_ATTRIBUTE_VALUE_INVALID, {"use", raw}));
        if (definition.hasAttribute("default") && usage != "optional")
            fail(definition, i18n::format(i18n::Message::SCHEMA_DEFAULT_WITH_USE, {raw}));
        checkAttributeParts(definition);
        checkAttributeName(definition, source, global);
    }

    /*
    ** Части объявления, которые исключают друг друга (src-attribute.3, 4):
    ** ровно одно из name и ref; у ссылки ref нет form, type и вложенного
    ** xs:simpleType — всё это у глобального объявления; type и вложенный
    ** xs:simpleType вместе нельзя.
    **
    ** The parts of a declaration that exclude each other (src-attribute.3, 4):
    ** exactly one of name and ref; a ref reference has no form, type and nested
    ** xs:simpleType — all of that belongs to the global declaration; type and a nested
    ** xs:simpleType together are not allowed.
    */
    void SchemaBuilder::checkAttributeParts(const Element& definition) const
    {
        static const std::string_view REFERENCE_FORBIDDEN[] = {"name", "form", "type"};
        std::string name = attributeOr(definition, "name", attributeOr(definition, "ref", ""));
        bool anonymous = firstSchemaChild(definition, "simpleType") != nullptr;

        if (!definition.hasAttribute("name") && !definition.hasAttribute("ref"))
            fail(definition, i18n::format(i18n::Message::SCHEMA_ATTRIBUTE_NO_NAME));
        for (std::string_view attribute : REFERENCE_FORBIDDEN)
        {
            if (definition.hasAttribute("ref") && definition.hasAttribute(attribute))
                fail(definition, i18n::format(i18n::Message::SCHEMA_ATTRIBUTE_PARTS_CONFLICT, {name, "ref", attribute}));
        }
        if (definition.hasAttribute("ref") && anonymous)
            fail(definition, i18n::format(i18n::Message::SCHEMA_ATTRIBUTE_PARTS_CONFLICT, {name, "ref", "xs:simpleType"}));
        if (definition.hasAttribute("type") && anonymous)
            fail(definition, i18n::format(i18n::Message::SCHEMA_ATTRIBUTE_PARTS_CONFLICT, {name, "type", "xs:simpleType"}));
    }

    /*
    ** Имя объявленного атрибута (не ссылки): NCName, не xmlns (no-xmlns), и
    ** пространство имён объявления — не xsi (no-xsi, 3.2.6): атрибуты
    ** xmlns и xsi:* принадлежат самому XML и XML Schema.
    **
    ** The name of a declared attribute (not a reference): an NCName, not xmlns (no-xmlns), and
    ** the namespace of the declaration is not xsi (no-xsi, 3.2.6): the attributes
    ** xmlns and xsi:* belong to XML and XML Schema themselves.
    */
    void SchemaBuilder::checkAttributeName(const Element& definition, const SchemaSource& source, bool global) const
    {
        std::optional<std::string> name = definition.getAttribute("name");
        std::string namespaceUri = source.targetNamespace;

        if (!name.has_value())
            return;
        std::string value = normalizeWhiteSpace(name.value(), WhiteSpace::COLLAPSE);
        checkNCName(definition, "name", value);
        if (!global)
            namespaceUri = localNamespace(definition, source, true);
        if (value == "xmlns" || namespaceUri == XSI_NAMESPACE_URI)
            fail(definition, i18n::format(i18n::Message::SCHEMA_ATTRIBUTE_RESERVED_NAME, {value}));
    }
}
