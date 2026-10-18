#include "xsd/internal/datatype_checks.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Часть длительности: последовательность «число + обозначение», где
    ** обозначения идут строго в порядке designators (YMD или HMS).
    ** Дробное число допустимо только у секунд (S).
    **
    ** A part of a duration: a sequence of "number + designator", where the
    ** designators go strictly in the order of designators (YMD or HMS).
    ** A fractional number is allowed only for seconds (S).
    */
    static bool parseDurationPart(std::string_view part, std::string_view designators) noexcept
    {
        std::size_t position = 0;
        std::size_t next = 0;

        while (position < part.size())
        {
            std::size_t start = position;
            bool fraction = false;
            while (position < part.size() && ((part[position] >= '0' && part[position] <= '9')
                || (part[position] == '.' && !fraction)))
            {
                fraction = fraction || part[position] == '.';
                ++position;
            }
            if (position == start || position >= part.size())
                return false;
            std::size_t found = designators.find(part[position], next);
            if (found == std::string_view::npos || (fraction && part[position] != 'S'))
                return false;
            next = found + 1;
            ++position;
        }
        return true;
    }

    /*
    ** duration: -?P[nY][nM][nD][T[nH][nM][n[.n]S]]. Нужен хотя бы один
    ** компонент; если есть 'T', после неё — хотя бы один из H, M, S.
    **
    ** duration: -?P[nY][nM][nD][T[nH][nM][n[.n]S]]. At least one
    ** component is needed; if there is a 'T', at least one of H, M, S follows it.
    */
    bool isDurationLexical(std::string_view value) noexcept
    {
        std::size_t position = 0;

        if (!value.empty() && value[0] == '-')
            position = 1;
        if (position >= value.size() || value[position] != 'P')
            return false;
        std::string_view rest = value.substr(position + 1);
        std::size_t separator = rest.find('T');
        std::string_view datePart = rest.substr(0, separator);
        std::string_view timePart;
        if (separator != std::string_view::npos)
        {
            timePart = rest.substr(separator + 1);
            if (timePart.empty())
                return false;
        }
        if (datePart.empty() && timePart.empty())
            return false;
        return parseDurationPart(datePart, "YMD") && parseDurationPart(timePart, "HMS");
    }
}
