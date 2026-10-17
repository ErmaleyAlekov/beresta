#include "xsd/internal/datatype_checks.hpp"
#include "utils/utf8.hpp"

namespace beresta::xsd::detail
{
    // Подходит ли значение под лексическое пространство примитива и встроенной формы
    static bool lexicalAccepted(const SimpleType& type, std::string_view value, const Element* context)
    {
        DateTimeValue moment;

        switch (type.primitive)
        {
            case Primitive::BOOLEAN:
                return isBooleanLexical(value);
            case Primitive::DECIMAL:
                return isDecimalLexical(value, type.form == LexicalForm::INTEGER);
            case Primitive::FLOAT: case Primitive::DOUBLE:
                return isFloatLexical(value);
            case Primitive::DURATION:
                return isDurationLexical(value);
            case Primitive::HEX_BINARY:
                return hexBinaryLength(value).has_value();
            case Primitive::BASE64_BINARY:
                return base64BinaryLength(value).has_value();
            case Primitive::QNAME: case Primitive::NOTATION_TYPE:
                return isResolvableQName(value, context);
            case Primitive::STRING: case Primitive::ANY_URI: case Primitive::ANY_SIMPLE:
                return matchesLexicalForm(type.form, value);
            default:
                return parseDateTimeValue(type.primitive, value, moment);
        }
    }

    // Лексическая проверка с понятным сообщением
    std::optional<std::string> checkLexical(const SimpleType& type, std::string_view value,
        const Element* context)
    {
        if (lexicalAccepted(type, value, context))
            return std::nullopt;
        return "значение \"" + std::string(value) + "\" не соответствует типу " + type.name;
    }

    // Длина: в символах для строк, в байтах для двоичных типов
    std::optional<std::string> checkLengthFacets(const SimpleType& type, std::string_view value)
    {
        const Facets& facets = type.facets;
        std::size_t length = utils::countCodePoints(value);

        if (type.primitive == Primitive::HEX_BINARY)
            length = hexBinaryLength(value).value_or(0);
        if (type.primitive == Primitive::BASE64_BINARY)
            length = base64BinaryLength(value).value_or(0);
        if (facets.length.has_value() && length != facets.length.value())
            return "длина значения " + std::to_string(length) + " вместо " + std::to_string(*facets.length);
        if (facets.minLength.has_value() && length < facets.minLength.value())
            return "длина значения " + std::to_string(length) + " меньше " + std::to_string(*facets.minLength);
        if (facets.maxLength.has_value() && length > facets.maxLength.value())
            return "длина значения " + std::to_string(length) + " больше " + std::to_string(*facets.maxLength);
        return std::nullopt;
    }

    // Описание одной границы для таблицы проверок
    struct RangeBound
    {
        const std::optional<std::string>& value;
        bool lower;
        bool inclusive;
        const char* message;
    };

    /*
    ** Границы min/max Inclusive/Exclusive. Несравнимые значения (NaN, типы
    ** без порядка) границы не нарушают — так ведёт себя частичный порядок XSD.
    */
    std::optional<std::string> checkRangeFacets(const SimpleType& type, std::string_view value)
    {
        const Facets& facets = type.facets;
        const RangeBound bounds[] = {
            {facets.minInclusive, true, true, "меньше минимума "},
            {facets.maxInclusive, false, true, "больше максимума "},
            {facets.minExclusive, true, false, "должно быть больше "},
            {facets.maxExclusive, false, false, "должно быть меньше "}
        };

        for (const RangeBound& bound : bounds)
        {
            if (!bound.value.has_value())
                continue;
            int order = compareOrdered(type.primitive, value, bound.value.value());
            if (breaksBound(order, bound.lower, bound.inclusive))
                return "значение \"" + std::string(value) + "\" " + bound.message + bound.value.value();
        }
        return std::nullopt;
    }

    // totalDigits / fractionDigits — только для десятичных типов
    std::optional<std::string> checkDigitFacets(const SimpleType& type, std::string_view value)
    {
        std::size_t total = 0;
        std::size_t fraction = 0;

        if (type.primitive != Primitive::DECIMAL)
            return std::nullopt;
        countDecimalDigits(value, total, fraction);
        if (type.facets.totalDigits.has_value() && total > type.facets.totalDigits.value())
            return "в числе " + std::string(value) + " больше " + std::to_string(*type.facets.totalDigits)
                + " значащих цифр";
        if (type.facets.fractionDigits.has_value() && fraction > type.facets.fractionDigits.value())
            return "в числе " + std::string(value) + " больше " + std::to_string(*type.facets.fractionDigits)
                + " цифр после точки";
        return std::nullopt;
    }
}
