#include "conformance/canonical_writer.hpp"

namespace conformance
{
    namespace
    {
        // Позиция после ведущих инструкций обработки <?...?> (0 — если их нет)
        // The position after the leading processing instructions <?...?> (0 if there are none)
        std::size_t skipLeadingInstructions(const std::string& text)
        {
            std::size_t position = 0;

            while (text.compare(position, 2, "<?") == 0)
            {
                std::size_t end = text.find("?>", position);

                if (end == std::string::npos)
                    return position;
                position = end + 2;
            }
            return position;
        }
    }

    namespace detail
    {
        /*
        ** Экранирование канонической формы: семь символов заменяются
        ** ссылками, остальные байты UTF-8 копируются как есть.
        **
        ** Escaping of the canonical form: seven characters are replaced with
        ** references, the other UTF-8 bytes are copied as is.
        */
        void appendCanonicalEscaped(std::string_view text, std::string& output)
        {
            for (char symbol : text)
            {
                if (symbol == '&')
                    output += "&amp;";
                else if (symbol == '<')
                    output += "&lt;";
                else if (symbol == '>')
                    output += "&gt;";
                else if (symbol == '"')
                    output += "&quot;";
                else if (symbol == '\t')
                    output += "&#9;";
                else if (symbol == '\n')
                    output += "&#10;";
                else if (symbol == '\r')
                    output += "&#13;";
                else
                    output += symbol;
            }
        }

        /*
        ** Вторая каноническая форма содержит блок
        **   <!DOCTYPE doc [\n<!NOTATION ...>\n]>\n
        ** Он срезается целиком вместе с завершающим переводом строки.
        ** Выводы IBM вдобавок выносят ПЕРЕД DOCTYPE инструкции обработки
        ** из самого DTD; у DOM для них нет места, поэтому при dropDtdPis
        ** срезаются и ведущие PI (вызывающий пробует оба варианта: PI
        ** в прологе документа DOM обязан сохранить).
        **
        ** The second canonical form contains the block
        **   <!DOCTYPE doc [\n<!NOTATION ...>\n]>\n
        ** It is cut off whole together with the trailing line break.
        ** The IBM outputs in addition move the processing instructions of the DTD itself
        ** BEFORE the DOCTYPE; the DOM has no place for them, so with dropDtdPis
        ** the leading PIs are cut off too (the caller tries both variants: the DOM
        ** must keep PIs in the document prolog).
        */
        std::string stripCanonicalDoctype(const std::string& expected, bool dropDtdPis)
        {
            std::size_t start = skipLeadingInstructions(expected);

            if (expected.compare(start, 9, "<!DOCTYPE") != 0)
                return expected;
            std::size_t end = expected.find("]>", start);
            if (end == std::string::npos)
                return expected;
            end += 2;
            if (end < expected.size() && expected[end] == '\n')
                ++end;
            if (dropDtdPis)
                return expected.substr(end);
            return expected.substr(0, start) + expected.substr(end);
        }
    }
}
