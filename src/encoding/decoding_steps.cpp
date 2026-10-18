#include "encoding/internal/decoding_steps.hpp"
#include "i18n/i18n.hpp"
#include "exception/parse_exception.hpp"
#include "parser/xml_scanner.hpp"
#include "utils/string_utils.hpp"
#include "utils/utf8.hpp"

#include <utility>

namespace beresta::encoding::detail
{
    namespace
    {
        // Семейство кодировки: все варианты UTF-16 (LE, BE, без суффикса) — одно семейство
        // The encoding family: all UTF-16 variants (LE, BE, without a suffix) are one family
        std::string family(std::string_view name)
        {
            std::string result;

            for (char symbol : name)
                result += utils::toLowerAscii(symbol);
            if (result.compare(0, 6, "utf-16") == 0)
                return "utf-16";
            return result;
        }
    }

    // Позиция пересчитывается в строку и столбец тем же сканером, что и в парсере
    // The position is converted to a line and a column by the same scanner as in the parser
    void failEncoding(const std::string& message, std::string_view text, std::size_t offset)
    {
        beresta::detail::XmlScanner scanner(text);

        throw ParseException(message, scanner.computePosition(offset), DomErrorCode::ENCODING_ERROR);
    }

    // Декларация читается уже из UTF-8 текста; неизвестное имя здесь не ошибка (его отвергнет парсер)
    // The declaration is read from the UTF-8 text already; an unknown name is not an error here (the parser rejects it)
    void checkDeclaredEncoding(std::string_view text, const Charset& used)
    {
        std::string declared = readDeclaredEncoding(text);
        const Charset* named = findCharset(declared);

        if (declared.empty() || named == nullptr)
            return;
        if (family(named->getName()) != family(used.getName()))
            failEncoding(i18n::format(i18n::Message::ENCODING_BOM_MISMATCH, {used.getName(), declared}), text, 0);
    }

    // Текст проверяется до нормализации: смещение ошибки указывает на исходный символ
    // The text is checked before normalization: the error offset points to the source character
    DecodedDocument finishDecoding(std::string_view body, std::string converted,
        const Charset& charset, bool checkDeclaration)
    {
        bool isUtf8 = &charset == &utf8Charset();
        std::string_view text = converted;
        std::string name(charset.getName());

        if (isUtf8)
            text = body;
        std::size_t invalid = utils::findInvalidXmlChar(text);
        if (invalid != std::string_view::npos)
            failEncoding(i18n::format(i18n::Message::ENCODING_INVALID_UTF8), text, invalid);
        if (checkDeclaration)
            checkDeclaredEncoding(text, charset);
        if (text.find('\r') == std::string_view::npos)
            return DecodedDocument(body, std::move(converted), !isUtf8, name);
        return DecodedDocument(body, utils::normalizeLineEndings(text), true, name);
    }
}
