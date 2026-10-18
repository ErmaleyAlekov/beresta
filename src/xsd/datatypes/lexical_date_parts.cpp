#include "xsd/internal/datatype_checks.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Год: необязательный минус и не меньше 4 цифр; при более чем 4 цифрах
    ** ведущий ноль запрещён. Год 0000 в XSD 1.0 не существует.
    **
    ** The year: an optional minus and at least 4 digits; with more than 4 digits
    ** a leading zero is forbidden. The year 0000 does not exist in XSD 1.0.
    */
    bool readYear(std::string_view text, std::size_t& position, long long& year) noexcept
    {
        bool negative = position < text.size() && text[position] == '-';
        std::size_t start = position;

        if (negative)
            start = ++position;
        year = 0;
        while (position < text.size() && text[position] >= '0' && text[position] <= '9')
        {
            if (position - start > 12)
                return false;
            year = year * 10 + (text[position] - '0');
            ++position;
        }
        std::size_t count = position - start;
        if (count < 4 || (count > 4 && text[start] == '0') || year == 0)
            return false;
        if (negative)
            year = -year;
        return true;
    }

    // Ровно count цифр
    // Exactly count digits
    bool readDigits(std::string_view text, std::size_t& position, std::size_t count, int& value) noexcept
    {
        value = 0;
        for (std::size_t index = 0; index < count; ++index, ++position)
        {
            if (position >= text.size() || text[position] < '0' || text[position] > '9')
                return false;
            value = value * 10 + (text[position] - '0');
        }
        return true;
    }

    // Секунды: две цифры и необязательная дробная часть (.цифры)
    // Seconds: two digits and an optional fraction (.digits)
    bool readSeconds(std::string_view text, std::size_t& position, long double& seconds) noexcept
    {
        int whole = 0;
        long double scale = 0.1L;

        if (!readDigits(text, position, 2, whole))
            return false;
        seconds = whole;
        if (position >= text.size() || text[position] != '.')
            return true;
        std::size_t start = ++position;
        while (position < text.size() && text[position] >= '0' && text[position] <= '9')
        {
            seconds += (text[position] - '0') * scale;
            scale /= 10.0L;
            ++position;
        }
        return position > start;
    }

    // Часовой пояс: Z или ±hh:mm (не более 14:00); отсутствие пояса допустимо
    // The time zone: Z or ±hh:mm (at most 14:00); no time zone is allowed
    bool readTimezone(std::string_view text, std::size_t& position, DateTimeValue& result) noexcept
    {
        int hours = 0;
        int minutes = 0;

        if (position >= text.size())
            return true;
        result.hasTimezone = true;
        if (text[position] == 'Z')
        {
            ++position;
            return true;
        }
        bool negative = text[position] == '-';
        if (!negative && text[position] != '+')
            return false;
        ++position;
        if (!readDigits(text, position, 2, hours) || position >= text.size() || text[position++] != ':')
            return false;
        if (!readDigits(text, position, 2, minutes) || minutes > 59 || hours * 60 + minutes > 14 * 60)
            return false;
        result.timezoneMinutes = hours * 60 + minutes;
        if (negative)
            result.timezoneMinutes = -result.timezoneMinutes;
        return true;
    }
}
