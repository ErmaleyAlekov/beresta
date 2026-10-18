#ifndef XML_TOKENS_HPP
#define XML_TOKENS_HPP

#include <string_view>
#include <vector>

namespace beresta::utils
{
    // Делит строку по пробельным символам на непустые токены (без копирования)
    // Splits a string by whitespace into non-empty tokens (without copying)
    std::vector<std::string_view> splitTokens(std::string_view value);

    // Nmtoken — непустая последовательность символов имени (может начинаться с цифры)
    // Nmtoken — a non-empty sequence of name characters (may start with a digit)
    bool isValidNmtoken(std::string_view token) noexcept;

    /*
    ** Делит строку ТОЛЬКО по пробелам #x20 (пустые куски отбрасываются).
    ** Так делятся значения атрибутов списковых типов после нормализации
    ** (раздел 3.3.3): таб или перевод строки из ссылки &#9; остаётся внутри
    ** токена и делает его недопустимым, как и требует стандарт.
    **
    ** Splits a string ONLY by #x20 spaces (empty pieces are dropped).
    ** This is how values of list-type attributes are split after normalization
    ** (section 3.3.3): a tab or a line feed from a &#9; reference stays inside
    ** the token and makes it invalid, as the standard requires.
    */
    std::vector<std::string_view> splitOnSpaces(std::string_view value);

    // Публичный идентификатор: только символы PubidChar (продукция [13] XML 1.0)
    // A public identifier: only PubidChar characters (production [13] of XML 1.0)
    bool isValidPublicId(std::string_view text) noexcept;
}

#endif
