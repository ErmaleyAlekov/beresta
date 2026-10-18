#include "encoding/encoding_detector.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::encoding
{
    // Сравнение начала байтовой строки с сигнатурой
    // Compares the start of a byte string with a signature
    static bool startsWithBytes(std::string_view bytes, std::string_view signature) noexcept
    {
        return bytes.substr(0, signature.size()) == signature;
    }

    // Порядок проверок важен: сначала BOM, затем сигнатуры UTF-16 без BOM
    // The order of checks matters: first the BOM, then the UTF-16 signatures without a BOM
    DetectedEncoding detectEncoding(std::string_view bytes)
    {
        using namespace std::string_view_literals;

        if (startsWithBytes(bytes, "\xEF\xBB\xBF"sv))
            return DetectedEncoding{"UTF-8", 3};
        if (startsWithBytes(bytes, "\xFF\xFE"sv))
            return DetectedEncoding{"UTF-16LE", 2};
        if (startsWithBytes(bytes, "\xFE\xFF"sv))
            return DetectedEncoding{"UTF-16BE", 2};
        if (startsWithBytes(bytes, "<\0?\0"sv))
            return DetectedEncoding{"UTF-16LE", 0};
        if (startsWithBytes(bytes, "\0<\0?"sv))
            return DetectedEncoding{"UTF-16BE", 0};
        std::string declared = readDeclaredEncoding(bytes);
        if (declared.empty())
            return DetectedEncoding{"UTF-8", 0};
        return DetectedEncoding{declared, 0};
    }

    /*
    ** Декларация читается как ASCII: все поддерживаемые однобайтовые
    ** кодировки совпадают с ASCII в первой половине таблицы.
    ** Ищем encoding = "..." или '...' внутри первых "<?xml ... ?>".
    ** Декларация — только "<?xml" и сразу пробел, как и в парсере: иначе
    ** инструкция "<?xmlA encoding='...'?>" (найдено фаззингом) меняла бы
    ** кодировку документа, у которого декларации нет вовсе.
    **
    ** The declaration is read as ASCII: all supported single-byte
    ** encodings match ASCII in the first half of the table.
    ** It looks for encoding = "..." or '...' inside the first "<?xml ... ?>".
    ** The declaration is only "<?xml" immediately followed by a space, as in the parser: otherwise
    ** the instruction "<?xmlA encoding='...'?>" (found by fuzzing) would change
    ** the encoding of a document that has no declaration at all.
    */
    std::string readDeclaredEncoding(std::string_view bytes)
    {
        if (!startsWithBytes(bytes, "<?xml") || bytes.size() < 6 || !utils::isWhitespace(bytes[5]))
            return std::string();
        std::string_view declaration = bytes.substr(0, bytes.find("?>"));
        std::size_t position = declaration.find("encoding");
        if (position == std::string_view::npos)
            return std::string();
        position += 8;
        while (position < declaration.size() && utils::isWhitespace(declaration[position]))
            ++position;
        if (position >= declaration.size() || declaration[position] != '=')
            return std::string();
        ++position;
        while (position < declaration.size() && utils::isWhitespace(declaration[position]))
            ++position;
        if (position >= declaration.size())
            return std::string();
        char quote = declaration[position];
        std::size_t end = declaration.find(quote, position + 1);
        if ((quote != '"' && quote != '\'') || end == std::string_view::npos)
            return std::string();
        return std::string(declaration.substr(position + 1, end - position - 1));
    }
}
