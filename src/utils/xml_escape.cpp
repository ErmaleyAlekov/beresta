#include "utils/xml_escape.hpp"
#include "utils/string_utils.hpp"

namespace beresta::utils
{
    // Символы, требующие экранирования в текстовом узле
    // Characters that require escaping in a text node
    static constexpr std::string_view TEXT_SPECIAL_CHARS = "&<>\r";

    // Символы, требующие экранирования в значении атрибута (кавычка — всегда двойная)
    // Characters that require escaping in an attribute value (the quote is always double)
    static constexpr std::string_view ATTRIBUTE_SPECIAL_CHARS = "&<>\"\t\n\r";

    // Экранирование текстового узла — частный случай общего алгоритма
    // Escaping a text node is a special case of the common algorithm
    void appendEscapedText(std::string& output, std::string_view text)
    {
        appendEscaped(output, text, TEXT_SPECIAL_CHARS);
    }

    // Экранирование значения атрибута — частный случай общего алгоритма
    // Escaping an attribute value is a special case of the common algorithm
    void appendEscapedAttribute(std::string& output, std::string_view value)
    {
        appendEscaped(output, value, ATTRIBUTE_SPECIAL_CHARS);
    }

    /*
    ** Ищем ближайший спецсимвол через findAny и копируем весь
    ** «чистый» кусок до него одной операцией append. Это заметно быстрее
    ** посимвольного копирования и не создаёт временных строк.
    **
    ** Finds the nearest special character with findAny and copies the whole
    ** "clean" piece before it in one append operation. This is noticeably faster
    ** than copying character by character and creates no temporary strings.
    */
    void appendEscaped(std::string& output, std::string_view text,
        std::string_view specialChars)
    {
        std::size_t position = 0;

        while (position < text.size())
        {
            std::size_t special = findAny(text, position, specialChars);
            if (special == std::string_view::npos)
            {
                output.append(text.substr(position));
                return;
            }
            output.append(text.substr(position, special - position));
            output.append(escapeSequenceFor(text[special]));
            position = special + 1;
        }
    }

    // Таблица подстановок для всех спецсимволов, встречающихся в обоих наборах
    // The substitution table for all special characters occurring in both sets
    std::string_view escapeSequenceFor(char symbol) noexcept
    {
        switch (symbol)
        {
            case '&':
                return "&amp;";
            case '<':
                return "&lt;";
            case '>':
                return "&gt;";
            case '"':
                return "&quot;";
            case '\t':
                return "&#9;";
            case '\n':
                return "&#10;";
            case '\r':
                return "&#13;";
            default:
                return "";
        }
    }
}
