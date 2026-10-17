#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_functions.hpp"
#include "utils/utf8.hpp"
#include "utils/xml_tokens.hpp"

#include <cmath>

namespace beresta::xpath::detail
{
    // substring-after(строка, разделитель): часть после первого вхождения
    XPathValue functionSubstringAfter(const FunctionCall& call)
    {
        std::string text = stringArgument(call, 0);
        std::string separator = stringArgument(call, 1);
        std::size_t found = text.find(separator);

        if (found == std::string::npos)
            return XPathValue::fromString("");
        return XPathValue::fromString(text.substr(found + separator.size()));
    }

    /*
    ** substring(строка, начало, длина?) — позиции символов с единицы.
    ** Берутся символы p с round(начало) <= p < round(начало) + round(длина);
    ** сравнения с NaN ложны, поэтому крайние случаи работают по спецификации.
    */
    XPathValue functionSubstring(const FunctionCall& call)
    {
        std::vector<std::uint32_t> codes = toCodePoints(stringArgument(call, 0));
        double start = std::floor(numberArgument(call, 1) + 0.5);
        double end = HUGE_VAL;
        std::size_t first = codes.size();
        std::size_t last = 0;

        if (call.arguments.size() > 2)
            end = start + std::floor(numberArgument(call, 2) + 0.5);
        for (std::size_t index = 0; index < codes.size(); ++index)
        {
            double position = static_cast<double>(index + 1);
            if (position >= start && position < end)
            {
                if (first == codes.size())
                    first = index;
                last = index + 1;
            }
        }
        return XPathValue::fromString(fromCodePoints(codes, first, last));
    }

    // string-length(строка?): длина в символах, а не в байтах
    XPathValue functionStringLength(const FunctionCall& call)
    {
        return XPathValue::fromNumber(
            static_cast<double>(utils::countCodePoints(stringArgument(call, 0))));
    }

    // normalize-space(строка?): пробелы по краям убираются, внутри — сжимаются до одного
    XPathValue functionNormalizeSpace(const FunctionCall& call)
    {
        std::string text = stringArgument(call, 0);
        std::string result;

        for (std::string_view token : utils::splitTokens(text))
        {
            if (!result.empty())
                result += ' ';
            result.append(token);
        }
        return XPathValue::fromString(result);
    }

    /*
    ** translate(строка, откуда, куда): символ из «откуда» заменяется
    ** символом с той же позицией в «куда» или удаляется, если там его нет.
    */
    XPathValue functionTranslate(const FunctionCall& call)
    {
        std::vector<std::uint32_t> text = toCodePoints(stringArgument(call, 0));
        std::vector<std::uint32_t> from = toCodePoints(stringArgument(call, 1));
        std::vector<std::uint32_t> to = toCodePoints(stringArgument(call, 2));
        std::string result;

        for (std::uint32_t code : text)
        {
            std::size_t position = 0;
            while (position < from.size() && from[position] != code)
                ++position;
            if (position == from.size())
                utils::appendUtf8(result, code);
            else if (position < to.size())
                utils::appendUtf8(result, to[position]);
        }
        return XPathValue::fromString(result);
    }
}
