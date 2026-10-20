#include "xsd/internal/datatype_checks.hpp"
#include "i18n/i18n.hpp"
#include "utils/utf8.hpp"

namespace beresta::xsd::detail
{
    // Подходит ли значение под лексическое пространство примитива и встроенной формы
    // Whether the value fits the lexical space of the primitive and the built-in form
    static bool lexicalAccepted(const SimpleType& type, std::string_view value, const PrefixResolver& prefixes)
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
                return isResolvableQName(value, prefixes);
            case Primitive::STRING: case Primitive::ANY_URI: case Primitive::ANY_SIMPLE:
                return matchesLexicalForm(type.form, value);
            default:
                return parseDateTimeValue(type.primitive, value, moment);
        }
    }

    // Лексическая проверка с понятным сообщением
    // A lexical check with a clear message
    std::optional<std::string> checkLexical(const SimpleType& type, std::string_view value,
        const PrefixResolver& prefixes)
    {
        if (lexicalAccepted(type, value, prefixes))
            return std::nullopt;
        return i18n::format(i18n::Message::VALUE_NOT_OF_TYPE, {value, type.name});
    }

    // Длина: в символах для строк, в байтах для двоичных типов
    // The length: in characters for strings, in bytes for binary types
    std::optional<std::string> checkLengthFacets(const SimpleType& type, std::string_view value)
    {
        const Facets& facets = type.facets;

        // Без фасетов длины не нужно и считать символы (длинный текст — лишний проход)
        // Without length facets there is no need to count the characters (a long text is an extra pass)
        if (!facets.length.has_value() && !facets.minLength.has_value() && !facets.maxLength.has_value())
            return std::nullopt;
        std::size_t length = utils::countCodePoints(value);

        if (type.primitive == Primitive::HEX_BINARY)
            length = hexBinaryLength(value).value_or(0);
        if (type.primitive == Primitive::BASE64_BINARY)
            length = base64BinaryLength(value).value_or(0);
        if (facets.length.has_value() && length != facets.length.value())
            return i18n::format(i18n::Message::LENGTH_NOT_EQUAL, {std::to_string(length), std::to_string(*facets.length)});
        if (facets.minLength.has_value() && length < facets.minLength.value())
            return i18n::format(i18n::Message::LENGTH_TOO_SMALL, {std::to_string(length), std::to_string(*facets.minLength)});
        if (facets.maxLength.has_value() && length > facets.maxLength.value())
            return i18n::format(i18n::Message::LENGTH_TOO_LARGE, {std::to_string(length), std::to_string(*facets.maxLength)});
        return std::nullopt;
    }

    // Описание одной границы для таблицы проверок
    // The description of one bound for the check table
    struct RangeBound
    {
        const std::optional<std::string>& value;
        bool lower;
        bool inclusive;
        i18n::Message message;
    };

    /*
    ** Границы min/max Inclusive/Exclusive. Несравнимое с границей значение
    ** (NaN, длительность вне частичного порядка) её нарушает (breaksBound).
    **
    ** The min/max Inclusive/Exclusive bounds. A value incomparable with the bound
    ** (NaN, a duration outside the partial order) violates it (breaksBound).
    */
    std::optional<std::string> checkRangeFacets(const SimpleType& type, std::string_view value)
    {
        const Facets& facets = type.facets;
        const RangeBound bounds[] = {
            {facets.minInclusive, true, true, i18n::Message::BELOW_MIN_INCLUSIVE},
            {facets.maxInclusive, false, true, i18n::Message::ABOVE_MAX_INCLUSIVE},
            {facets.minExclusive, true, false, i18n::Message::NOT_ABOVE_MIN_EXCLUSIVE},
            {facets.maxExclusive, false, false, i18n::Message::NOT_BELOW_MAX_EXCLUSIVE}
        };

        for (const RangeBound& bound : bounds)
        {
            if (!bound.value.has_value())
                continue;
            int order = compareOrdered(type.primitive, value, bound.value.value());
            if (breaksBound(order, bound.lower, bound.inclusive))
                return i18n::format(bound.message, {value, bound.value.value()});
        }
        return std::nullopt;
    }

    // totalDigits / fractionDigits — только для десятичных типов
    // totalDigits / fractionDigits — only for decimal types
    std::optional<std::string> checkDigitFacets(const SimpleType& type, std::string_view value)
    {
        std::size_t total = 0;
        std::size_t fraction = 0;

        if (type.primitive != Primitive::DECIMAL)
            return std::nullopt;
        countDecimalDigits(value, total, fraction);
        if (type.facets.totalDigits.has_value() && total > type.facets.totalDigits.value())
            return i18n::format(i18n::Message::TOO_MANY_DIGITS, {value, std::to_string(*type.facets.totalDigits)});
        if (type.facets.fractionDigits.has_value() && fraction > type.facets.fractionDigits.value())
            return i18n::format(i18n::Message::TOO_MANY_FRACTION_DIGITS, {value, std::to_string(*type.facets.fractionDigits)});
        return std::nullopt;
    }
}
