#include "encoding/encoding_detector.hpp"
#include "i18n/i18n.hpp"
#include "encoding/charset.hpp"
#include "encoding/internal/decoding_steps.hpp"
#include "exception/parse_exception.hpp"

#include <utility>

namespace beresta::encoding
{
    // Хранит либо ссылку на исходный UTF-8, либо перекодированную копию
    // Keeps either a reference to the source UTF-8 or a transcoded copy
    DecodedDocument::DecodedDocument(std::string_view original, std::string converted,
        bool isConverted, std::string encodingName)
        : _original(original), _converted(std::move(converted)),
          _isConverted(isConverted), _encoding(std::move(encodingName))
    {
    }

    // Представление вычисляется при каждом вызове — поэтому перемещение объекта безопасно
    // The view is computed on every call — that is why moving the object is safe
    std::string_view DecodedDocument::getText() const noexcept
    {
        if (_isConverted)
            return _converted;
        return _original;
    }

    // Исходная кодировка
    // The source encoding
    const std::string& DecodedDocument::getEncoding() const noexcept
    {
        return _encoding;
    }

    /*
    ** UTF-8 не копируется — проверяется на месте. Другие кодировки
    ** перекодируются. Дальше (finishDecoding) текст проверяется на
    ** символы, запрещённые стандартом XML, и нормализуются концы строк.
    ** Декларация сверяется с кодировкой, только когда ту определили BOM
    ** или сигнатура UTF-16, а не сама декларация (раздел 4.3.3).
    **
    ** UTF-8 is not copied — it is checked in place. Other encodings
    ** are transcoded. Then (finishDecoding) the text is checked for
    ** characters forbidden by the XML standard, and line endings are normalized.
    ** The declaration is matched against the encoding only when the encoding was
    ** detected by a BOM or the UTF-16 signature, not by the declaration itself (section 4.3.3).
    */
    DecodedDocument decodeDocument(std::string_view bytes, std::string_view forcedEncoding)
    {
        DetectedEncoding detected = detectEncoding(bytes);
        std::string name = detected.name;
        if (!forcedEncoding.empty())
            name = std::string(forcedEncoding);
        const Charset* charset = findCharset(name);
        if (charset == nullptr)
            throw ParseException(i18n::format(i18n::Message::ENCODING_UNSUPPORTED, {name}),
                ParsePosition{0, 0, 0}, DomErrorCode::ENCODING_ERROR);
        std::string_view body = bytes.substr(detected.bomLength);
        std::string converted;
        if (charset != &utf8Charset() && charset->decode(body, converted) != CHARSET_OK)
            detail::failEncoding(i18n::format(i18n::Message::ENCODING_BAD_BYTE, {name}), converted, converted.size());
        bool bySignature = detected.bomLength > 0 || detected.name.compare(0, 6, "UTF-16") == 0;
        return detail::finishDecoding(body, std::move(converted), *charset,
            forcedEncoding.empty() && bySignature);
    }
}
