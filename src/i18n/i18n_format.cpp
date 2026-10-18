#include "i18n/i18n.hpp"

#include <cstddef>

namespace beresta::i18n
{
    /*
    ** Номер аргумента в метке {N} начиная с позиции открывающей скобки.
    ** Возвращает false, если это не метка (например, просто "{" в тексте);
    ** тогда символ выводится как есть.
    **
    ** The argument number of a {N} placeholder starting at the opening brace.
    ** Returns false if this is not a placeholder (for example, a plain "{"
    ** in the text); the character is then written as is.
    */
    static bool readPlaceholder(std::string_view text, std::size_t start, std::size_t& index,
        std::size_t& end) noexcept
    {
        std::size_t position = start + 1;

        index = 0;
        while (position < text.size() && text[position] >= '0' && text[position] <= '9')
        {
            index = index * 10 + static_cast<std::size_t>(text[position] - '0');
            ++position;
        }
        if (position == start + 1 || position >= text.size() || text[position] != '}')
            return false;
        end = position + 1;
        return true;
    }

    /*
    ** Подстановка аргументов в шаблон. Порядок меток в шаблоне свободный:
    ** в разных языках аргументы стоят в разных местах фразы ("элемент {0}
    ** не объявлен" / "undeclared element {0}"). Метка без аргумента
    ** остаётся в тексте как есть — так ошибка перевода видна сразу.
    **
    ** Substitutes arguments into a template. Placeholders may appear in any
    ** order: different languages put arguments in different places of a
    ** sentence. A placeholder without an argument stays in the text as is,
    ** so a translation mistake is visible at once.
    */
    static std::string substitute(std::string_view text, std::initializer_list<std::string_view> arguments)
    {
        std::string result;
        std::size_t position = 0;

        result.reserve(text.size() + 32);
        while (position < text.size())
        {
            std::size_t index = 0;
            std::size_t end = 0;
            bool placeholder = text[position] == '{' && readPlaceholder(text, position, index, end);
            if (placeholder && index < arguments.size())
            {
                result += *(arguments.begin() + index);
                position = end;
                continue;
            }
            result += text[position];
            ++position;
        }
        return result;
    }

    // Текст на текущем языке: поиск каталога, шаблона и подстановка аргументов
    // Text in the current language: find the catalog and the template, substitute arguments
    std::string format(Message id, std::initializer_list<std::string_view> arguments)
    {
        const Catalog* catalog = findCatalog(language());

        if (catalog == nullptr)
            catalog = &ENGLISH_CATALOG;
        return substitute(lookup(*catalog, id), arguments);
    }
}
