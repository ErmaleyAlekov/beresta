#include "xsd/internal/datatype_checks.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Формат каждого вида даты/времени: Y — год, M — месяц, D — день,
    ** h — часы, m — минуты, s — секунды; прочие символы — литералы.
    */
    static std::string_view dateFormat(Primitive primitive) noexcept
    {
        switch (primitive)
        {
            case Primitive::DATE_TIME:
                return "Y-M-DTh:m:s";
            case Primitive::DATE:
                return "Y-M-D";
            case Primitive::TIME:
                return "h:m:s";
            case Primitive::G_YEAR_MONTH:
                return "Y-M";
            case Primitive::G_YEAR:
                return "Y";
            case Primitive::G_MONTH_DAY:
                return "--M-D";
            case Primitive::G_DAY:
                return "---D";
            default:
                return "--M";
        }
    }

    // Один элемент формата: поле или литерал
    static bool readField(char field, std::string_view text, std::size_t& position,
        DateTimeValue& result) noexcept
    {
        if (field == 'Y')
            return readYear(text, position, result.year);
        if (field == 'M')
            return readDigits(text, position, 2, result.month);
        if (field == 'D')
            return readDigits(text, position, 2, result.day);
        if (field == 'h')
            return readDigits(text, position, 2, result.hour);
        if (field == 'm')
            return readDigits(text, position, 2, result.minute);
        if (field == 's')
            return readSeconds(text, position, result.second);
        if (position >= text.size() || text[position] != field)
            return false;
        ++position;
        return true;
    }

    // Число дней в месяце с учётом високосных лет (григорианский календарь)
    static int daysInMonth(long long year, int month) noexcept
    {
        static const int DAYS[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        if (month != 2)
            return DAYS[month - 1];
        bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
        if (leap)
            return 29;
        return 28;
    }

    // Диапазоны полей; 24:00:00 допустимо как полночь конца суток
    static bool fieldsInRange(Primitive primitive, const DateTimeValue& value) noexcept
    {
        if (value.month < 1 || value.month > 12 || value.minute > 59 || value.second >= 60.0L)
            return false;
        if (value.hour > 24 || (value.hour == 24 && (value.minute != 0 || value.second != 0.0L)))
            return false;
        bool hasYear = primitive == Primitive::DATE_TIME || primitive == Primitive::DATE;
        long long year = 2000;
        if (hasYear)
            year = value.year;
        return value.day >= 1 && value.day <= daysInMonth(year, value.month);
    }

    // Разбор по формату, затем необязательный часовой пояс и проверка диапазонов
    bool parseDateTimeValue(Primitive primitive, std::string_view value, DateTimeValue& result)
    {
        std::size_t position = 0;

        result = DateTimeValue();
        for (char field : dateFormat(primitive))
        {
            if (!readField(field, value, position, result))
                return false;
        }
        if (!readTimezone(value, position, result) || position != value.size())
            return false;
        return fieldsInRange(primitive, result);
    }
}
