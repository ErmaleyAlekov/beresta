#include "encoding/encoding_detector.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::encoding
{
    // Сравнение начала байтовой строки с сигнатурой
    static bool startsWithBytes(std::string_view bytes, std::string_view signature) noexcept
    {
        return bytes.substr(0, signature.size()) == signature;
    }

    // Порядок проверок важен: сначала BOM, затем сигнатуры UTF-16 без BOM
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
    */
    std::string readDeclaredEncoding(std::string_view bytes)
    {
        if (!startsWithBytes(bytes, "<?xml"))
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
