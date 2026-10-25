#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

#include <string>

namespace beresta::xsd::detail
{
    /*
    ** Значения default / fixed элементов проверяются после сборки всей схемы:
    ** при построении объявления его тип может быть ещё не достроен
    ** (рекурсивная модель), а тип, взятый у головы группы замены, уточняется
    ** в inheritHeadTypes. К этому моменту все встроенные и объявленные типы
    ** полны, и проверка не обращается к недостроенному типу.
    **
    ** The default / fixed values of elements are checked after the whole schema is
    ** built: while a declaration is being built its type may be incomplete yet
    ** (a recursive model), and a type taken from the substitution group head is
    ** refined in inheritHeadTypes. By this moment all the built-in and declared
    ** types are complete, and the check does not touch an incomplete type.
    */
    void SchemaBuilder::checkElementValues() const
    {
        for (const std::pair<const Element*, const ElementDecl*>& check : _valueChecks)
            checkValueConstraints(*check.first, check.second->type, check.second->defaultValue,
                check.second->fixedValue);
    }

    // default и fixed (что из них задано) проверяются по типу объявления
    // default and fixed (whichever is given) are checked by the type of the declaration
    void SchemaBuilder::checkValueConstraints(const Element& definition, TypeRef type,
        const std::optional<std::string>& defaultValue, const std::optional<std::string>& fixedValue) const
    {
        if (defaultValue.has_value())
            checkValueConstraint(definition, type, "default", defaultValue.value());
        if (fixedValue.has_value())
            checkValueConstraint(definition, type, "fixed", fixedValue.value());
    }

    /*
    ** Ограничение значения объявления (e-props-correct.2, a-props-correct.2,
    ** cos-valid-default):
    **  - составной тип со смешанным содержимым (и xs:anyType) принимает любую
    **    строку; с простым содержимым значение проверяется по этому простому
    **    типу; с пустым содержимым или только элементами ограничения быть не может;
    **  - иначе строка обязана быть допустимой записью простого типа; префиксы
    **    QName берутся из области видимости самого объявления в схеме.
    **
    ** A value constraint of a declaration (e-props-correct.2, a-props-correct.2,
    ** cos-valid-default):
    **  - a complex type with mixed content (and xs:anyType) accepts any string;
    **    with simple content the value is checked by that simple type; with empty
    **    or element-only content there can be no constraint;
    **  - otherwise the string must be a valid literal of the simple type; QName
    **    prefixes are taken from the scope of the declaration itself in the schema.
    */
    void SchemaBuilder::checkValueConstraint(const Element& definition, TypeRef type,
        std::string_view attribute, const std::string& value) const
    {
        std::string name = attributeOr(definition, "name", attributeOr(definition, "ref", ""));
        const SimpleType* simple = type.simple;

        if (type.complex != nullptr)
        {
            if (type.complex->content == ContentType::MIXED_CONTENT)
                return;
            simple = type.complex->simpleContent;
            if (type.complex->content != ContentType::SIMPLE_CONTENT || simple == nullptr)
                fail(definition, i18n::format(i18n::Message::SCHEMA_VALUE_CONSTRAINT_CONTENT, {name, attribute}));
        }
        if (simple == nullptr)
            return;
        std::optional<std::string> error = validateSimpleValue(*simple, value, &definition);
        if (error.has_value())
            fail(definition, i18n::format(i18n::Message::SCHEMA_VALUE_CONSTRAINT_INVALID,
                {attribute, value, name, error.value()}));
    }
}
