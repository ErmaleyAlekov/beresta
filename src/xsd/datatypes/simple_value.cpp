#include "xsd/internal/datatype_checks.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd
{
    /*
    ** Атомарное значение: лексика, затем фасеты длины, границ, цифр,
    ** шаблонов и перечисления. Возвращается первая найденная ошибка.
    **
    ** An atomic value: the lexis, then the length, bound, digit,
    ** pattern and enumeration facets. The first error found is returned.
    */
    static std::optional<std::string> validateAtomic(const SimpleType& type, std::string_view value,
        const Element* context)
    {
        std::optional<std::string> error = detail::checkLexical(type, value, context);

        if (!error)
            error = detail::checkLengthFacets(type, value);
        if (!error)
            error = detail::checkRangeFacets(type, value);
        if (!error)
            error = detail::checkDigitFacets(type, value);
        if (!error)
            error = detail::checkPatternsAndEnumeration(type, value);
        return error;
    }

    // Список: фасеты длины считают элементы, каждый элемент проверяется своим типом
    // A list: the length facets count the items, each item is checked by its type
    static std::optional<std::string> validateList(const SimpleType& type, std::string_view value,
        const Element* context)
    {
        std::vector<std::string_view> items = utils::splitTokens(value);
        const Facets& facets = type.facets;
        std::string count = std::to_string(items.size());

        if (facets.length.has_value() && items.size() != facets.length.value())
            return i18n::format(i18n::Message::LIST_LENGTH_NOT_EQUAL, {count, std::to_string(*facets.length)});
        if (facets.minLength.has_value() && items.size() < facets.minLength.value())
            return i18n::format(i18n::Message::LIST_TOO_SHORT, {count, std::to_string(*facets.minLength)});
        if (facets.maxLength.has_value() && items.size() > facets.maxLength.value())
            return i18n::format(i18n::Message::LIST_TOO_LONG, {count, std::to_string(*facets.maxLength)});
        for (std::string_view item : items)
        {
            std::optional<std::string> error = validateSimpleValue(*type.itemType, item, context);
            if (error)
                return error;
        }
        return detail::checkPatternsAndEnumeration(type, value);
    }

    // Точка входа: нормализация пробелов и выбор по разновидности типа
    // The entry point: whitespace normalization and the choice by the type variety
    std::optional<std::string> validateSimpleValue(const SimpleType& type, std::string_view value,
        const Element* context)
    {
        std::string buffer;
        std::string_view normalized = detail::normalizedView(value, type.facets.whiteSpace, buffer);

        if (type.variety == Variety::LIST)
            return validateList(type, normalized, context);
        if (type.variety == Variety::ATOMIC)
            return validateAtomic(type, normalized, context);
        for (const SimpleType* member : type.memberTypes)
        {
            if (!validateSimpleValue(*member, value, context))
                return detail::checkPatternsAndEnumeration(type, normalized);
        }
        return i18n::format(i18n::Message::UNION_NO_MEMBER, {normalized, type.name});
    }

    // Равенство в пространстве значений: числа и даты — по значению, остальное — по строке
    // Equality in the value space: numbers and dates by value, the rest by string
    bool valuesEqual(const SimpleType& type, std::string_view left, std::string_view right)
    {
        std::string leftBuffer;
        std::string rightBuffer;
        std::string_view a = detail::normalizedView(left, type.facets.whiteSpace, leftBuffer);
        std::string_view b = detail::normalizedView(right, type.facets.whiteSpace, rightBuffer);

        if (type.variety != Variety::ATOMIC)
            return a == b;
        if (type.primitive == Primitive::BOOLEAN)
            return (a == "true" || a == "1") == (b == "true" || b == "1");
        if (type.primitive == Primitive::DECIMAL && detail::isDecimalLexical(a, false))
            return detail::compareDecimal(a, b) == 0;
        bool ordered = type.primitive == Primitive::FLOAT || type.primitive == Primitive::DOUBLE
            || (type.primitive >= Primitive::DATE_TIME && type.primitive <= Primitive::G_MONTH);
        if (ordered)
            return detail::compareOrdered(type.primitive, a, b) == 0;
        return a == b;
    }
}
