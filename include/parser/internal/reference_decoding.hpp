#ifndef REFERENCE_DECODING_HPP
#define REFERENCE_DECODING_HPP

#include <optional>
#include <string>
#include <string_view>

namespace beresta::detail
{
    // Символ пяти предопределённых сущностей XML (lt, gt, amp, quot, apos)
    // The character of the five predefined XML entities (lt, gt, amp, quot, apos)
    std::optional<char> predefinedEntity(std::string_view name) noexcept;

    /*
    ** Символьная ссылка: body — текст между "&#" и ";" ("1046" или "x416").
    ** Код проверяется на допустимость в XML и дописывается в output в UTF-8.
    ** Возвращает false, если ссылка некорректна.
    **
    ** A character reference: body is the text between "&#" and ";" ("1046" or "x416").
    ** The code is checked for being allowed in XML and appended to output in UTF-8.
    ** Returns false if the reference is invalid.
    */
    bool decodeCharacterReference(std::string_view body, std::string& output);
}

#endif
