#ifndef DECODING_STEPS_HPP
#define DECODING_STEPS_HPP

#include "encoding/charset.hpp"
#include "encoding/encoding_detector.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace beresta::encoding::detail
{
    // Ошибка кодировки с позицией, вычисленной по уже декодированному тексту
    // An encoding error with the position computed from the already decoded text
    [[noreturn]] void failEncoding(const std::string& message, std::string_view text, std::size_t offset);

    /*
    ** Раздел 4.3.3: сущность в кодировке, отличной от названной в её
    ** декларации, — фатальная ошибка. Проверяется, когда кодировку
    ** определил BOM или сигнатура UTF-16 (иначе её и брали из декларации):
    ** BOM UTF-8 + encoding="iso-8859-1" или BOM UTF-16 + encoding="utf-8".
    **
    ** Section 4.3.3: an entity in an encoding other than the one named in its
    ** declaration is a fatal error. It is checked when the encoding was
    ** detected by a BOM or the UTF-16 signature (otherwise it was taken from the declaration):
    ** a UTF-8 BOM + encoding="iso-8859-1" or a UTF-16 BOM + encoding="utf-8".
    */
    void checkDeclaredEncoding(std::string_view text, const Charset& used);

    /*
    ** Хвост декодирования: проверка допустимых символов, (при check)
    ** согласия декларации с кодировкой и нормализация концов строк.
    ** converted пуст для UTF-8 — тогда текстом служит body без копии.
    **
    ** The decoding tail: the check of allowed characters, (with check) the
    ** agreement of the declaration with the encoding and line ending normalization.
    ** converted is empty for UTF-8 — then body serves as the text without a copy.
    */
    DecodedDocument finishDecoding(std::string_view body, std::string converted,
        const Charset& charset, bool checkDeclaration);
}

#endif
