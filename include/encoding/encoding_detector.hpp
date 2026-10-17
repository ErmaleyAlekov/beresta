#ifndef ENCODING_DETECTOR_HPP
#define ENCODING_DETECTOR_HPP

#include <cstddef>
#include <string>
#include <string_view>

namespace beresta::encoding
{
    // Результат автоопределения: имя кодировки и длина BOM, который нужно пропустить
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
    */
    DetectedEncoding detectEncoding(std::string_view bytes);

    // Значение encoding="..." из XML-декларации в ASCII-совместимом тексте ("" — нет)
    std::string readDeclaredEncoding(std::string_view bytes);

    /*
    ** Документ, приведённый к UTF-8. Если вход уже был в UTF-8, копии нет:
    ** getText() указывает прямо во входной буфер (он должен жить дольше
    ** объекта). Иначе перекодированный текст хранится внутри объекта.
    ** Из-за этого объект нельзя копировать — только перемещать.
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
        std::string_view getText() const noexcept;

        // Имя исходной кодировки
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
    */
    DecodedDocument decodeDocument(std::string_view bytes, std::string_view forcedEncoding);
}

#endif
