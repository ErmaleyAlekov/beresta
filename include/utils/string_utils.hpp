#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <string>
#include <string_view>

namespace beresta::utils
{
    // Возвращает представление строки без завершающих пробельных символов (без копирования)
    // Returns a view of the string without trailing whitespace (without copying)
    std::string_view trimRight(std::string_view text) noexcept;

    // Сравнивает две строки без учёта регистра латинских букв (ASCII)
    // Compares two strings ignoring the case of Latin letters (ASCII)
    bool equalsIgnoreCaseAscii(std::string_view left, std::string_view right) noexcept;

    // Переводит латинскую букву в нижний регистр, остальные символы не трогает
    // Converts a Latin letter to lower case, leaves other characters unchanged
    char toLowerAscii(char symbol) noexcept;

    /*
    ** Нормализация концов строк (раздел 2.11 XML 1.0): "\r\n" и одиночный
    ** "\r" превращаются в "\n". Выполняется ОДИН раз над целым входом —
    ** документом, внешним DTD, внешней сущностью — до разбора, чтобы
    ** символы \r, полученные из ссылок &#13;, сохранились как есть.
    **
    ** Line ending normalization (section 2.11 of XML 1.0): "\r\n" and a lone
    ** "\r" become "\n". It is done ONCE over the whole input —
    ** the document, an external DTD, an external entity — before parsing, so that
    ** \r characters obtained from &#13; references are kept as is.
    */
    std::string normalizeLineEndings(std::string_view text);

    /*
    ** Позиция первого символа из небольшого набора symbols, начиная с from
    ** (npos — не найден). Замена std::string_view::find_first_of для горячих
    ** путей: стандартная реализация вызывает memchr по набору на КАЖДЫЙ
    ** символ текста, а здесь — простые сравнения в одном цикле.
    **
    ** The position of the first character from a small set symbols, starting
    ** at from (npos — not found). A replacement for std::string_view::find_first_of
    ** on hot paths: the standard implementation calls memchr over the set for EACH
    ** text character, while here there are plain comparisons in one loop.
    */
    std::size_t findAny(std::string_view text, std::size_t from, std::string_view symbols) noexcept;
}

#endif
