#include "xsd/internal/datatype_checks.hpp"
#include "utils/decimal.hpp"

namespace beresta::xsd::detail {

    using utils::Decimal;

    // RU: РџРѕР»РЅС‹Рµ 400-Р»РµС‚РЅРёРµ С†РёРєР»С‹ РІС…РѕРґСЏС‚ РІ СЃРµРєСѓРЅРґС‹, РѕСЃС‚Р°С‚РѕРє С…СЂР°РЅРёС‚СЃСЏ РІ РјРµСЃСЏС†Р°С….
    // EN: Seconds include complete Gregorian cycles, leaving a bounded month offset.
    struct DurationParts {
        Decimal seconds = utils::decimalFromInteger(0);
        int months = 0;
    };

    // RU: Деление на 4800 оставляет точный остаток даже для длинных значений.
    // EN: Divide by 4800 to retain the exact remainder for arbitrarily long values.
    static void splitCycles(Decimal months, DurationParts& parts) {
        int remainder = 0;
        for (char& digit : months.digits) {
            const int value = remainder * 10 + (digit - '0');
            digit = static_cast<char>('0' + value / 4800);
            remainder = value % 4800;
        }
        Decimal cycles;
        utils::readDecimal(months.digits, cycles);
        parts.months = remainder;
        const auto cycleSeconds = utils::decimalFromInteger(146097LL * 86400);
        parts.seconds = utils::addDecimals(parts.seconds, utils::multiplyDecimals(cycles, cycleSeconds));
    }

    // RU: Накапливает месяцы и секунды отдельно до календарного сдвига.
    // EN: Keep months and seconds separate until the calendar shift.
    static void addComponent(char unit, bool inTime, Decimal value, Decimal& months, DurationParts& parts) {
        if (unit == 'Y' || (unit == 'M' && !inTime)) {
            if (unit == 'Y') value = utils::multiplyDecimals(value, utils::decimalFromInteger(12));
            months = utils::addDecimals(months, value);
            return;
        }
        int factor = 1;
        if (unit == 'D') factor = 86400;
        else if (unit == 'H') factor = 3600;
        else if (unit == 'M') factor = 60;
        parts.seconds = utils::addDecimals(parts.seconds,
            utils::multiplyDecimals(value, utils::decimalFromInteger(factor)));
    }

    // RU: Считывает величины из проверенной лексической формы длительности.
    // EN: Read component values from a validated duration literal.
    static bool parseDuration(std::string_view value, DurationParts& parts) {
        if (!isDurationLexical(value)) return false;
        Decimal months = utils::decimalFromInteger(0);
        bool inTime = false;
        std::size_t start = value.find('P') + 1;
        for (std::size_t i = start; i < value.size(); ++i) {
            const char symbol = value[i];
            if ((symbol >= '0' && symbol <= '9') || symbol == '.') continue;
            if (symbol == 'T') inTime = true;
            else {
                Decimal number;
                if (!utils::readDecimal(value.substr(start, i - start), number)) return false;
                addComponent(symbol, inTime, number, months, parts);
            }
            start = i + 1;
        }
        splitCycles(months, parts);
        if (value.front() == '-') {
            parts.months = -parts.months;
            parts.seconds = utils::negateDecimal(parts.seconds);
        }
        return true;
    }

    // RU: РћРїРѕСЂРЅС‹Рµ РґР°С‚С‹ РѕСЃС‚Р°СЋС‚СЃСЏ РІ РїРѕР»РѕР¶РёС‚РµР»СЊРЅС‹С… РіРѕРґР°С… РїСЂРё СЃРґРІРёРіРµ РґРѕ 4799 РјРµСЃСЏС†РµРІ.
    // EN: The reference years stay positive for residual shifts up to 4799 months.
    static Decimal shiftedMoment(int year, int month, const DurationParts& parts) {
        const int total = year * 12 + month - 1 + parts.months;
        const int shiftedMonth = total % 12 + 1;
        const int shiftedYear = total / 12 - (shiftedMonth <= 2);
        const int dayOfYear = (153 * ((shiftedMonth + 9) % 12) + 2) / 5;
        const int days = shiftedYear * 365 + shiftedYear / 4 - shiftedYear / 100
                       + shiftedYear / 400 + dayOfYear;
        return utils::addDecimals(utils::decimalFromInteger(days * 86400LL), parts.seconds);
    }

    // RU: Все четыре опорные даты должны дать один порядок.
    // EN: All four reference dates must agree on the ordering.
    int compareDurations(std::string_view left, std::string_view right) {
        static const int REFERENCES[4][2] = {{1696, 9}, {1697, 2}, {1903, 3}, {1903, 7}};
        DurationParts a;
        DurationParts b;
        if (!parseDuration(left, a) || !parseDuration(right, b)) return 2;
        int agreed = 3;
        for (const auto& date : REFERENCES) {
            const int order = utils::compareDecimals(shiftedMoment(date[0], date[1], a),
                                                      shiftedMoment(date[0], date[1], b));
            if (agreed != 3 && agreed != order) return 2;
            agreed = order;
        }
        return agreed;
    }

} // namespace beresta::xsd::detail
