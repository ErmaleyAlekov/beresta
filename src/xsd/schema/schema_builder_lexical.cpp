#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Логический атрибут схемы (abstract, nillable, mixed) — значение
    ** типа xs:boolean: true / 1 или false / 0 с пробелами по краям.
    ** Любое другое написание ("True", "", "yes") — ошибка схемы:
    ** раньше оно молча считалось false.
    **
    ** A boolean schema attribute (abstract, nillable, mixed) is a value
    ** of the xs:boolean type: true / 1 or false / 0 with spaces at the edges.
    ** Any other spelling ("True", "", "yes") is a schema error:
    ** previously it was silently taken as false.
    */
    bool SchemaBuilder::booleanAttribute(const Element& definition, std::string_view name, bool fallback) const
    {
        std::optional<std::string> value = definition.getAttribute(name);

        if (!value.has_value())
            return fallback;
        std::vector<std::string_view> tokens = utils::splitTokens(value.value());
        std::string_view text;
        if (tokens.size() == 1)
            text = tokens.front();
        if (text == "true" || text == "1")
            return true;
        if (text == "false" || text == "0")
            return false;
        fail(definition, i18n::format(i18n::Message::SCHEMA_ATTRIBUTE_VALUE_INVALID, {name, value.value()}));
    }

    /*
    ** Атрибуты, которых не может быть у xs:element в данном месте
    ** (схема для схем и src-element.2.2):
    **  - глобальное объявление — без ref, form, minOccurs, maxOccurs;
    **  - локальное — без abstract, final, substitutionGroup;
    **  - ссылка ref — только minOccurs, maxOccurs и id.
    **
    ** The attributes an xs:element cannot have in the given place
    ** (the schema for schemas and src-element.2.2):
    **  - a global declaration — without ref, form, minOccurs, maxOccurs;
    **  - a local one — without abstract, final, substitutionGroup;
    **  - a ref reference — only minOccurs, maxOccurs and id.
    */
    static std::vector<std::string_view> forbiddenAttributes(ElementPlace place)
    {
        if (place == ElementPlace::GLOBAL_ELEMENT)
            return {"ref", "form", "minOccurs", "maxOccurs"};
        if (place == ElementPlace::LOCAL_ELEMENT)
            return {"abstract", "final", "substitutionGroup"};
        return {"name", "type", "nillable", "default", "fixed", "form", "block", "abstract", "final",
            "substitutionGroup"};
    }

    // Сообщение о запрещённой части объявления — своё для каждого места
    // The message about a forbidden part of a declaration — its own for each place
    static std::string forbiddenMessage(ElementPlace place, std::string_view element, std::string_view part)
    {
        i18n::Message message = i18n::Message::SCHEMA_ELEMENT_REF_FORBIDDEN;

        if (place == ElementPlace::GLOBAL_ELEMENT)
            message = i18n::Message::SCHEMA_GLOBAL_ELEMENT_FORBIDDEN;
        else if (place == ElementPlace::LOCAL_ELEMENT)
            message = i18n::Message::SCHEMA_LOCAL_ELEMENT_FORBIDDEN;
        return i18n::format(message, {element, part});
    }

    /*
    ** Проверка формы объявления xs:element. У ссылки ref, кроме того,
    ** не может быть собственного типа и ограничений идентичности:
    ** всё это принадлежит глобальному объявлению, на которое она ссылается.
    **
    ** Checking the form of an xs:element declaration. A ref reference, in addition,
    ** cannot have its own type and identity constraints:
    ** all of that belongs to the global declaration it refers to.
    */
    void SchemaBuilder::checkElementAttributes(const Element& definition, ElementPlace place) const
    {
        static const std::string_view REFERENCE_CHILDREN[] = {"complexType", "simpleType", "key", "keyref", "unique"};
        std::string element = attributeOr(definition, "name", attributeOr(definition, "ref", ""));

        for (std::string_view attribute : forbiddenAttributes(place))
        {
            if (definition.hasAttribute(attribute))
                fail(definition, forbiddenMessage(place, element, attribute));
        }
        if (place != ElementPlace::ELEMENT_REFERENCE)
            return;
        for (std::string_view child : REFERENCE_CHILDREN)
        {
            if (firstSchemaChild(definition, child) != nullptr)
                fail(definition, forbiddenMessage(place, element, "xs:" + std::string(child)));
        }
    }
}
