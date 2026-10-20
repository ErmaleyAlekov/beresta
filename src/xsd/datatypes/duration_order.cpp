#include "xsd/internal/datatype_checks.hpp"

#include <cmath>

namespace beresta::xsd::detail
{
    /*
    ** Значение длительности для сравнения: месяцы (годы * 12 + месяцы) и
    ** секунды (дни, часы, минуты и секунды). Обе части со знаком длительности.
    ** long double — потому что лексика не ограничивает величину чисел.
    **
    ** A duration value for comparison: months (years * 12 + months) and
    ** seconds (days, hours, minutes and seconds). Both parts carry the duration sign.
    ** long double — because the lexical form does not bound the size of the numbers.
    */
    struct DurationParts
    {
        long double months = 0.0L;
        long double seconds = 0.0L;
    };

    // Сколько единиц (месяцев или секунд) в одном обозначении; inTime — после 'T'
    // How many units (months or seconds) one designator stands for; inTime — after 'T'
    static void addComponent(DurationParts& parts, char designator, bool inTime, long double number) noexcept
    {
        if (designator == 'Y')
            parts.months += number * 12.0L;
        else if (designator == 'M' && !inTime)
            parts.months += number;
        else if (designator == 'D')
            parts.seconds += number * 86400.0L;
        else if (designator == 'H')
            parts.seconds += number * 3600.0L;
        else if (designator == 'M')
            parts.seconds += number * 60.0L;
        else
            parts.seconds += number;
    }

    // Очередная цифра числа: до точки — целая часть, после (scale > 0) — дробная
    // The next digit of a number: before the point — the integer part, after it (scale > 0) — the fraction
    static void addDigit(char symbol, long double& number, long double& scale) noexcept
    {
        if (scale == 0.0L)
        {
            number = number * 10.0L + (symbol - '0');
            return;
        }
        scale /= 10.0L;
        number += (symbol - '0') * scale;
    }

    /*
    ** Разбор уже проверенной лексики (isDurationLexical): числа копятся
    ** цифра за цифрой, буква-обозначение отдаёт накопленное число в addComponent.
    **
    ** Parsing an already checked lexical form (isDurationLexical): numbers accumulate
    ** digit by digit, a designator letter hands the accumulated number to addComponent.
    */
    static bool parseDuration(std::string_view value, DurationParts& parts) noexcept
    {
        if (!isDurationLexical(value))
            return false;
        bool negative = value[0] == '-';
        bool inTime = false;
        long double number = 0.0L;
        long double scale = 0.0L;
        for (char symbol : value)
        {
            if (symbol >= '0' && symbol <= '9')
                addDigit(symbol, number, scale);
            else if (symbol == '.')
                scale = 1.0L;
            else if (symbol == 'T')
                inTime = true;
            else if (symbol != '-' && symbol != 'P')
            {
                addComponent(parts, symbol, inTime, number);
                number = 0.0L;
                scale = 0.0L;
            }
        }
        if (negative)
            parts = DurationParts{-parts.months, -parts.seconds};
        return true;
    }

    /*
    ** Момент «опорная дата (1-е число месяца) + длительность» в секундах от
    ** 1970-01-01 (пролептический григорианский календарь, алгоритм days_from_civil
    ** Г. Хиннанта). Сначала прибавляются месяцы, затем секунды: день опорной даты —
    ** первое число, поэтому «прижимать» день к концу месяца (приложение E) не нужно,
    ** а прибавление дней и секунд с переносами равно прибавлению на прямой времени.
    **
    ** The moment "reference date (the 1st of a month) + duration" in seconds since
    ** 1970-01-01 (the proleptic Gregorian calendar, H. Hinnant's days_from_civil
    ** algorithm). Months are added first, then seconds: the reference day is the
    ** first of the month, so the day need not be "pinned" to the month end (appendix E),
    ** and adding days and seconds with carries equals adding on the time line.
    */
    static long double shiftedMoment(long double year, int month, const DurationParts& parts) noexcept
    {
        long double total = year * 12.0L + (month - 1) + parts.months;
        long double shiftedYear = std::floor(total / 12.0L);
        long double shiftedMonth = total - shiftedYear * 12.0L + 1.0L;

        if (shiftedMonth <= 2.0L)
            shiftedYear -= 1.0L;
        long double era = std::floor(shiftedYear / 400.0L);
        long double yearOfEra = shiftedYear - era * 400.0L;
        long double monthIndex = std::fmod(shiftedMonth + 9.0L, 12.0L);
        long double dayOfYear = std::floor((153.0L * monthIndex + 2.0L) / 5.0L);
        long double dayOfEra = yearOfEra * 365.0L + std::floor(yearOfEra / 4.0L)
            - std::floor(yearOfEra / 100.0L) + dayOfYear;
        long double days = era * 146097.0L + dayOfEra - 719468.0L;
        return days * 86400.0L + parts.seconds;
    }

    /*
    ** Частичный порядок duration (XSD 1.0, 3.2.6.2): длительности прибавляются
    ** к четырём опорным датам 1696-09-01, 1697-02-01, 1903-03-01 и 1903-07-01
    ** (на них расходятся длины месяцев и високосные годы). Порядок определён,
    ** только если все четыре сравнения согласны; иначе — 2 («несравнимы»).
    **
    ** The partial order of duration (XSD 1.0, 3.2.6.2): the durations are added
    ** to the four reference dates 1696-09-01, 1697-02-01, 1903-03-01 and 1903-07-01
    ** (where month lengths and leap years differ). The order is defined
    ** only if all four comparisons agree; otherwise 2 ("incomparable").
    */
    int compareDurations(std::string_view left, std::string_view right) noexcept
    {
        static const int REFERENCES[4][2] = {{1696, 9}, {1697, 2}, {1903, 3}, {1903, 7}};
        DurationParts a;
        DurationParts b;
        int agreed = 3;

        if (!parseDuration(left, a) || !parseDuration(right, b))
            return 2;
        for (const int* reference : REFERENCES)
        {
            long double difference = shiftedMoment(reference[0], reference[1], a)
                - shiftedMoment(reference[0], reference[1], b);
            int order = 0;
            if (difference < 0.0L)
                order = -1;
            else if (difference > 0.0L)
                order = 1;
            if (agreed != 3 && agreed != order)
                return 2;
            agreed = order;
        }
        return agreed;
    }
}
