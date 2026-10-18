#include "xsd/internal/datatype_checks.hpp"

namespace beresta::xsd::detail
{
    // Разбор уже проверенной лексики decimal на части
    // Splits already checked decimal lexis into parts
    DecimalParts splitDecimal(std::string_view value) noexcept
    {
        DecimalParts parts;

        if (!value.empty() && (value[0] == '-' || value[0] == '+'))
        {
            parts.negative = value[0] == '-';
            value.remove_prefix(1);
        }
        std::size_t point = value.find('.');
        parts.integer = value.substr(0, point);
        if (point != std::string_view::npos)
            parts.fraction = value.substr(point + 1);
        while (!parts.integer.empty() && parts.integer.front() == '0')
            parts.integer.remove_prefix(1);
        while (!parts.fraction.empty() && parts.fraction.back() == '0')
            parts.fraction.remove_suffix(1);
        if (parts.integer.empty() && parts.fraction.empty())
            parts.negative = false;
        return parts;
    }

    // Знак результата сравнения: -1, 0 или 1
    // The sign of a comparison result: -1, 0 or 1
    static int signOf(int value) noexcept
    {
        if (value < 0)
            return -1;
        if (value > 0)
            return 1;
        return 0;
    }

    // Сравнение модулей: длина целой части, затем цифры целой и дробной частей
    // Comparing absolute values: the length of the integer part, then the digits of the integer and fractional parts
    static int compareMagnitude(const DecimalParts& left, const DecimalParts& right) noexcept
    {
        if (left.integer.size() != right.integer.size())
        {
            if (left.integer.size() < right.integer.size())
                return -1;
            return 1;
        }
        int order = left.integer.compare(right.integer);
        if (order == 0)
            order = left.fraction.compare(right.fraction);
        return signOf(order);
    }

    /*
    ** Точное сравнение десятичных чисел любой длины без преобразования
    ** в double: сначала знак, затем модуль (у отрицательных порядок обратный).
    **
    ** An exact comparison of decimal numbers of any length without converting
    ** to double: first the sign, then the absolute value (reversed for negative numbers).
    */
    int compareDecimal(std::string_view left, std::string_view right)
    {
        DecimalParts a = splitDecimal(left);
        DecimalParts b = splitDecimal(right);

        if (a.negative != b.negative)
        {
            if (a.negative)
                return -1;
            return 1;
        }
        int result = compareMagnitude(a, b);
        if (a.negative)
            return -result;
        return result;
    }

    // Число и дробные цифры: для totalDigits / fractionDigits
    // The digit and fraction digit counts: for totalDigits / fractionDigits
    void countDecimalDigits(std::string_view value, std::size_t& total, std::size_t& fraction)
    {
        DecimalParts parts = splitDecimal(value);

        fraction = parts.fraction.size();
        total = parts.integer.size() + parts.fraction.size();
        if (total == 0)
            total = 1;
    }
}
