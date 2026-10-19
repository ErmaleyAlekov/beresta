#include "xsd/internal/datatype_checks.hpp"
#include "utils/decimal.hpp"

namespace beresta::xsd::detail
{
    // Число и дробные цифры: для totalDigits / fractionDigits
    // The digit and fraction digit counts: for totalDigits / fractionDigits
    void countDecimalDigits(std::string_view value, std::size_t& total, std::size_t& fraction)
    {
        utils::DecimalParts parts = utils::splitDecimal(value);

        fraction = parts.fraction.size();
        total = parts.integer.size() + parts.fraction.size();
        if (total == 0)
            total = 1;
    }
}
