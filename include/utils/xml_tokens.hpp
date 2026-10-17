#ifndef XML_TOKENS_HPP
#define XML_TOKENS_HPP

#include <string_view>
#include <vector>

namespace beresta::utils
{
    // Делит строку по пробельным символам на непустые токены (без копирования)
    std::vector<std::string_view> splitTokens(std::string_view value);

    // Nmtoken — непустая последовательность символов имени (может начинаться с цифры)
    bool isValidNmtoken(std::string_view token) noexcept;
}

#endif
