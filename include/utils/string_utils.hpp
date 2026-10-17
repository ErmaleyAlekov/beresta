#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <string_view>

namespace beresta::utils
{
    // Возвращает представление строки без завершающих пробельных символов (без копирования)
    std::string_view trimRight(std::string_view text) noexcept;

    // Сравнивает две строки без учёта регистра латинских букв (ASCII)
    bool equalsIgnoreCaseAscii(std::string_view left, std::string_view right) noexcept;

    // Переводит латинскую букву в нижний регистр, остальные символы не трогает
    char toLowerAscii(char symbol) noexcept;
}

#endif
