#include "xsd/internal/datatype_checks.hpp"

namespace beresta::xsd::detail
{
    // Пробельный символ XML: пробел, табуляция, перевод строки, возврат каретки
    // An XML whitespace character: space, tab, line feed, carriage return
    static bool isSpaceSymbol(char symbol) noexcept
    {
        return symbol == ' ' || symbol == '\t' || symbol == '\n' || symbol == '\r';
    }

    /*
    ** Уже ли значение имеет нормальный вид по правилу mode. Почти все
    ** значения в реальных документах уже нормальны, и такая проверка (один
    ** проход без выделения памяти) позволяет не копировать их вовсе.
    ** REPLACE: нет \t, \n, \r. COLLAPSE: вдобавок нет пробелов по краям и
    ** двух пробелов подряд.
    **
    ** Whether the value is already in the normal form by the mode rule. Almost all
    ** values in real documents are already normal, and this check (one
    ** pass without memory allocation) lets us not copy them at all.
    ** REPLACE: no \t, \n, \r. COLLAPSE: in addition no spaces at the edges and
    ** no two spaces in a row.
    */
    static bool isNormalized(std::string_view value, WhiteSpace mode) noexcept
    {
        if (mode == WhiteSpace::PRESERVE)
            return true;
        for (std::size_t index = 0; index < value.size(); ++index)
        {
            char symbol = value[index];
            if (symbol == '\t' || symbol == '\n' || symbol == '\r')
                return false;
            if (mode != WhiteSpace::COLLAPSE || symbol != ' ')
                continue;
            if (index == 0 || index + 1 == value.size() || value[index + 1] == ' ')
                return false;
        }
        return true;
    }

    /*
    ** COLLAPSE за один проход: серия пробельных символов превращается в
    ** один пробел, но только между словами — в начале и в конце она
    ** отбрасывается. pending — «видели пробелы после слова».
    **
    ** COLLAPSE in one pass: a run of whitespace characters turns into
    ** one space, but only between words — at the beginning and at the end it
    ** is dropped. pending means "we have seen spaces after a word".
    */
    static void collapseInto(std::string_view value, std::string& buffer)
    {
        bool pending = false;

        buffer.clear();
        buffer.reserve(value.size());
        for (char symbol : value)
        {
            if (isSpaceSymbol(symbol))
            {
                pending = !buffer.empty();
                continue;
            }
            if (pending)
                buffer += ' ';
            pending = false;
            buffer += symbol;
        }
    }

    /*
    ** Нормализованное значение без лишних копий: если value уже нормально,
    ** возвращается оно само, иначе результат строится в buffer и
    ** возвращается представление buffer. Вызывающий держит buffer живым,
    ** пока пользуется результатом.
    **
    ** The normalized value without extra copies: if value is already normal,
    ** it is returned itself, otherwise the result is built in buffer and
    ** a view of buffer is returned. The caller keeps buffer alive
    ** while using the result.
    */
    std::string_view normalizedView(std::string_view value, WhiteSpace mode, std::string& buffer)
    {
        if (isNormalized(value, mode))
            return value;
        if (mode == WhiteSpace::COLLAPSE)
        {
            collapseInto(value, buffer);
            return buffer;
        }
        buffer.assign(value);
        for (char& symbol : buffer)
        {
            if (isSpaceSymbol(symbol))
                symbol = ' ';
        }
        return buffer;
    }
}

namespace beresta::xsd
{
    // Пробелы по правилу whiteSpace: REPLACE заменяет, COLLAPSE ещё и сжимает
    // Whitespace by the whiteSpace rule: REPLACE replaces, COLLAPSE also collapses
    std::string normalizeWhiteSpace(std::string_view value, WhiteSpace mode)
    {
        std::string buffer;
        return std::string(detail::normalizedView(value, mode, buffer));
    }
}
