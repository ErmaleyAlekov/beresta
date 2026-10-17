#include "xsd/internal/datatype_checks.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd
{
    /*
    ** Атомарное значение: лексика, затем фасеты длины, границ, цифр,
    ** шаблонов и перечисления. Возвращается первая найденная ошибка.
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
    static std::optional<std::string> validateList(const SimpleType& type, std::string_view value,
        const Element* context)
    {
        std::vector<std::string_view> items = utils::splitTokens(value);
        const Facets& facets = type.facets;
        std::string count = std::to_string(items.size());

        if (facets.length.has_value() && items.size() != facets.length.value())
            return "в списке " + count + " элементов вместо " + std::to_string(*facets.length);
        if (facets.minLength.has_value() && items.size() < facets.minLength.value())
            return "в списке " + count + " элементов, меньше " + std::to_string(*facets.minLength);
        if (facets.maxLength.has_value() && items.size() > facets.maxLength.value())
            return "в списке " + count + " элементов, больше " + std::to_string(*facets.maxLength);
        for (std::string_view item : items)
        {
            std::optional<std::string> error = validateSimpleValue(*type.itemType, item, context);
            if (error)
                return error;
        }
        return detail::checkPatternsAndEnumeration(type, value);
    }

    // Пробелы по правилу whiteSpace: REPLACE заменяет, COLLAPSE ещё и сжимает
    std::string normalizeWhiteSpace(std::string_view value, WhiteSpace mode)
    {
        std::string result(value);

        if (mode == WhiteSpace::PRESERVE)
            return result;
        for (char& symbol : result)
        {
            if (symbol == '\t' || symbol == '\n' || symbol == '\r')
                symbol = ' ';
        }
        if (mode == WhiteSpace::REPLACE)
            return result;
        std::string collapsed;
        for (std::string_view token : utils::splitTokens(result))
        {
            if (!collapsed.empty())
                collapsed += ' ';
            collapsed.append(token);
        }
        return collapsed;
    }

    // Точка входа: нормализация пробелов и выбор по разновидности типа
    std::optional<std::string> validateSimpleValue(const SimpleType& type, std::string_view value,
        const Element* context)
    {
        std::string normalized = normalizeWhiteSpace(value, type.facets.whiteSpace);

        if (type.variety == Variety::LIST)
            return validateList(type, normalized, context);
        if (type.variety == Variety::ATOMIC)
            return validateAtomic(type, normalized, context);
        for (const SimpleType* member : type.memberTypes)
        {
            if (!validateSimpleValue(*member, value, context))
                return detail::checkPatternsAndEnumeration(type, normalized);
        }
        return "значение \"" + normalized + "\" не подходит ни под один тип объединения " + type.name;
    }

    // Равенство в пространстве значений: числа и даты — по значению, остальное — по строке
    bool valuesEqual(const SimpleType& type, std::string_view left, std::string_view right)
    {
        std::string a = normalizeWhiteSpace(left, type.facets.whiteSpace);
        std::string b = normalizeWhiteSpace(right, type.facets.whiteSpace);

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
