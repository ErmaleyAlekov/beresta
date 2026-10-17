#include "encoding/encoding_detector.hpp"
#include "encoding/charset.hpp"
#include "exception/parse_exception.hpp"
#include "parser/xml_scanner.hpp"
#include "utils/string_utils.hpp"
#include "utils/utf8.hpp"

#include <utility>

namespace beresta::encoding
{
    // Хранит либо ссылку на исходный UTF-8, либо перекодированную копию
    DecodedDocument::DecodedDocument(std::string_view original, std::string converted,
        bool isConverted, std::string encodingName)
        : _original(original), _converted(std::move(converted)),
          _isConverted(isConverted), _encoding(std::move(encodingName))
    {
    }

    // Представление вычисляется при каждом вызове — поэтому перемещение объекта безопасно
    std::string_view DecodedDocument::getText() const noexcept
    {
        if (_isConverted)
            return _converted;
        return _original;
    }

    // Исходная кодировка
    const std::string& DecodedDocument::getEncoding() const noexcept
    {
        return _encoding;
    }

    // Ошибка кодировки с позицией, вычисленной по уже декодированному тексту
    [[noreturn]] static void failEncoding(const std::string& message, std::string_view text,
        std::size_t offset)
    {
        detail::XmlScanner scanner(text);

        throw ParseException(message, scanner.computePosition(offset), DomErrorCode::ENCODING_ERROR);
    }

    /*
    ** UTF-8 не копируется — проверяется на месте. Другие кодировки
    ** перекодируются. В обоих случаях итоговый текст проверяется на
    ** символы, запрещённые стандартом XML (управляющие коды и т.п.).
    */
    DecodedDocument decodeDocument(std::string_view bytes, std::string_view forcedEncoding)
    {
        DetectedEncoding detected = detectEncoding(bytes);
        std::string name = detected.name;
        if (!forcedEncoding.empty())
            name = std::string(forcedEncoding);
        const Charset* charset = findCharset(name);
        if (charset == nullptr)
            throw ParseException("неподдерживаемая кодировка \"" + name + "\"",
                ParsePosition{0, 0, 0}, DomErrorCode::ENCODING_ERROR);
        std::string_view body = bytes.substr(detected.bomLength);
        bool isUtf8 = charset == &utf8Charset();
        std::string converted;
        if (!isUtf8 && charset->decode(body, converted) != CHARSET_OK)
            failEncoding("байт не соответствует кодировке " + name, converted, converted.size());
        DecodedDocument result(body, std::move(converted), !isUtf8, std::string(charset->getName()));
        std::size_t invalid = utils::findInvalidXmlChar(result.getText());
        if (invalid != std::string_view::npos)
            failEncoding("некорректная последовательность UTF-8 или запрещённый в XML символ",
                result.getText(), invalid);
        return result;
    }
}
