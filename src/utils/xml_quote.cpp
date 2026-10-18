#include "utils/xml_escape.hpp"

namespace beresta::utils
{
    /*
    ** Литералы DOCTYPE (системный и публичный идентификаторы) не поддерживают
    ** экранирование, поэтому кавычка выбирается по содержимому:
    ** двойная, если её нет внутри, иначе одинарная.
    **
    ** DOCTYPE literals (the system and public identifiers) do not support
    ** escaping, so the quote is chosen by the content:
    ** double if there is none inside, otherwise single.
    */
    char chooseQuote(std::string_view value) noexcept
    {
        if (value.find('"') == std::string_view::npos)
            return '"';
        if (value.find('\'') == std::string_view::npos)
            return '\'';
        return '\0';
    }

    // Значение в выбранных кавычках
    // The value in the chosen quotes
    void appendQuoted(std::string& output, std::string_view value)
    {
        char quote = chooseQuote(value);

        output += quote;
        output.append(value);
        output += quote;
    }
}
