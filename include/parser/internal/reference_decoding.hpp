#ifndef REFERENCE_DECODING_HPP
#define REFERENCE_DECODING_HPP

#include <optional>
#include <string>
#include <string_view>

namespace beresta::detail
{
    // Символ пяти предопределённых сущностей XML (lt, gt, amp, quot, apos)
    std::optional<char> predefinedEntity(std::string_view name) noexcept;

    /*
    ** Символьная ссылка: body — текст между "&#" и ";" ("1046" или "x416").
    ** Код проверяется на допустимость в XML и дописывается в output в UTF-8.
    ** Возвращает false, если ссылка некорректна.
    */
    bool decodeCharacterReference(std::string_view body, std::string& output);

    // Убирает текстовую декларацию <?xml ...?> в начале внешней сущности или DTD
    std::string_view stripTextDeclaration(std::string_view text) noexcept;
}

#endif
