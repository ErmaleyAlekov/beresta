#include "utils/internal/decimal_digits.hpp"

#include <vector>

namespace beresta::utils::detail
{
    /*
    ** Сравнение модулей: без ведущих нулей более длинное число больше,
    ** при равной длине порядок совпадает с порядком строк цифр.
    **
    ** Comparing absolute values: without leading zeros the longer number is greater,
    ** at equal length the order matches the order of the digit strings.
    */
    int compareDigits(std::string_view left, std::string_view right) noexcept
    {
        std::string_view a = stripLeadingZeros(left);
        std::string_view b = stripLeadingZeros(right);
        int order = 0;

        if (a.size() != b.size())
        {
            if (a.size() < b.size())
                return -1;
            return 1;
        }
        order = a.compare(b);
        if (order < 0)
            return -1;
        if (order > 0)
            return 1;
        return 0;
    }

    /*
    ** Сложение столбиком от младших разрядов с переносом. Результат
    ** собирается задом наперёд и разворачивается в конце.
    **
    ** Long-hand addition from the least significant digits with a carry. The result
    ** is collected backwards and reversed at the end.
    */
    std::string addDigits(std::string_view left, std::string_view right)
    {
        std::string reversed;
        std::size_t length = left.size();
        int carry = 0;

        if (right.size() > length)
            length = right.size();
        for (std::size_t index = 0; index < length || carry != 0; ++index)
        {
            int sum = carry;
            if (index < left.size())
                sum += left[left.size() - 1 - index] - '0';
            if (index < right.size())
                sum += right[right.size() - 1 - index] - '0';
            reversed.push_back(static_cast<char>('0' + sum % 10));
            carry = sum / 10;
        }
        std::string result(reversed.rbegin(), reversed.rend());
        return std::string(stripLeadingZeros(result));
    }

    /*
    ** Вычитание столбиком с заёмом (уменьшаемое не меньше вычитаемого,
    ** поэтому заём после старшего разряда не остаётся).
    **
    ** Long-hand subtraction with a borrow (the minuend is not less than the subtrahend,
    ** so no borrow remains after the most significant digit).
    */
    std::string subtractDigits(std::string_view left, std::string_view right)
    {
        std::string reversed;
        int borrow = 0;

        for (std::size_t index = 0; index < left.size(); ++index)
        {
            int difference = left[left.size() - 1 - index] - '0' - borrow;
            if (index < right.size())
                difference -= right[right.size() - 1 - index] - '0';
            borrow = 0;
            if (difference < 0)
            {
                difference += 10;
                borrow = 1;
            }
            reversed.push_back(static_cast<char>('0' + difference));
        }
        std::string result(reversed.rbegin(), reversed.rend());
        return std::string(stripLeadingZeros(result));
    }

    /*
    ** Умножение столбиком: произведения цифр копятся в разрядах (младший —
    ** первым), переносы раскладываются одним проходом в конце.
    **
    ** Long-hand multiplication: the products of digits accumulate in the places (the least
    ** significant first), the carries are spread in one pass at the end.
    */
    std::string multiplyDigits(std::string_view left, std::string_view right)
    {
        std::vector<unsigned long long> places(left.size() + right.size() + 1, 0);
        std::string reversed;
        unsigned long long carry = 0;

        for (std::size_t i = 0; i < left.size(); ++i)
        {
            for (std::size_t j = 0; j < right.size(); ++j)
                places[i + j] += static_cast<unsigned long long>((left[left.size() - 1 - i] - '0')
                    * (right[right.size() - 1 - j] - '0'));
        }
        for (unsigned long long place : places)
        {
            unsigned long long total = place + carry;
            reversed.push_back(static_cast<char>('0' + total % 10));
            carry = total / 10;
        }
        std::string result(reversed.rbegin(), reversed.rend());
        return std::string(stripLeadingZeros(result));
    }
}
