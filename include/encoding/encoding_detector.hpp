#ifndef ENCODING_DETECTOR_HPP
#define ENCODING_DETECTOR_HPP

#include <cstddef>
#include <string>
#include <string_view>

namespace beresta::encoding
{
    // Результат автоопределения: имя кодировки и длина BOM, который нужно пропустить
    // The detection result: the encoding name and the length of the BOM to skip
    struct DetectedEncoding
    {
        std::string name;
        std::size_t bomLength;
    };

    /*
    ** Определение кодировки по правилам приложения F спецификации XML:
    ** 1) BOM: EF BB BF (UTF-8), FF FE (UTF-16LE), FE FF (UTF-16BE);
    ** 2) без BOM — по первым байтам "<?" в UTF-16 (3C 00 3F 00 / 00 3C 00 3F);
    ** 3) иначе — по атрибуту encoding в XML-декларации; по умолчанию UTF-8.
    **
    ** Encoding detection by the rules of appendix F of the XML specification:
    ** 1) BOM: EF BB BF (UTF-8), FF FE (UTF-16LE), FE FF (UTF-16BE);
    ** 2) without a BOM — by the first bytes "<?" in UTF-16 (3C 00 3F 00 / 00 3C 00 3F);
    ** 3) otherwise — by the encoding attribute of the XML declaration; UTF-8 by default.
    */
    DetectedEncoding detectEncoding(std::string_view bytes);

    // Значение encoding="..." из XML-декларации в ASCII-совместимом тексте ("" — нет)
    // The encoding="..." value from the XML declaration in ASCII-compatible text ("" — none)
    std::string readDeclaredEncoding(std::string_view bytes);

    /*
    ** Документ, приведённый к UTF-8. Если вход уже был в UTF-8, копии нет:
    ** getText() указывает прямо во входной буфер (он должен жить дольше
    ** объекта). Иначе перекодированный текст хранится внутри объекта.
    ** Из-за этого объект нельзя копировать — только перемещать.
    **
    ** A document converted to UTF-8. If the input was already UTF-8, there is no copy:
    ** getText() points straight into the input buffer (it must outlive the
    ** object). Otherwise the transcoded text is kept inside the object.
    ** Because of this the object cannot be copied — only moved.
    */
    class DecodedDocument
    {
    public:
        DecodedDocument(std::string_view original, std::string converted,
            bool isConverted, std::string encodingName);
        DecodedDocument(const DecodedDocument&) = delete;
        DecodedDocument& operator=(const DecodedDocument&) = delete;
        DecodedDocument(DecodedDocument&&) = default;

        // Текст документа в UTF-8 (без BOM)
        // The document text in UTF-8 (without a BOM)
        std::string_view getText() const noexcept;

        // Имя исходной кодировки
        // The source encoding name
        const std::string& getEncoding() const noexcept;

    private:
        std::string_view _original;
        std::string _converted;
        bool _isConverted;
        std::string _encoding;
    };

    /*
    ** Полный конвейер: определить кодировку (или взять forcedEncoding),
    ** перекодировать в UTF-8 и проверить, что каждый символ допустим в XML.
    ** Ошибки — ParseException с кодом ENCODING_ERROR и позицией проблемы.
    **
    ** The full pipeline: detect the encoding (or take forcedEncoding),
    ** transcode to UTF-8 and check that every character is allowed in XML.
    ** Errors are ParseException with the ENCODING_ERROR code and the problem position.
    */
    DecodedDocument decodeDocument(std::string_view bytes, std::string_view forcedEncoding);
}

#endif
