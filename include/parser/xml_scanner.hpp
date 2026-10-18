#ifndef XML_SCANNER_HPP
#define XML_SCANNER_HPP

#include "exception/parse_exception.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace beresta::detail
{
    /*
    ** XmlScanner — лексический курсор по входному тексту.
    ** Хранит только std::string_view на исходные данные и текущую позицию,
    ** поэтому все операции чтения (readName, readUntil и т.д.) возвращают
    ** представления-срезы исходной строки БЕЗ выделения памяти.
    ** Строки (std::string) создаются только в момент построения узлов.
    **
    ** Сканер не зависит от формата: переводы строк и отступы — обычные
    ** символы, поэтому «красивый» и «сплошной» XML обрабатываются одинаково.
    **
    ** Строка и столбец вычисляются инкрементально: сканер помнит последнюю
    ** вычисленную позицию и при следующем запросе досчитывает только новый
    ** участок. Запросы по возрастанию смещений стоят O(n) в сумме.
    **
    ** «Якорь» (setErrorAnchor) используется для текста раскрытых сущностей:
    ** ошибки внутри сущности сообщаются в позиции ссылки на неё в документе.
    **
    ** XmlScanner — a lexical cursor over the input text.
    ** Keeps only a std::string_view of the source data and the current position,
    ** so all read operations (readName, readUntil and so on) return
    ** views (slices) of the source string WITHOUT memory allocation.
    ** Strings (std::string) are created only when nodes are built.
    ** The scanner does not depend on the format: line breaks and indentation are ordinary
    ** characters, so "pretty" and "solid" XML are handled the same way.
    ** The line and column are computed incrementally: the scanner remembers the last
    ** computed position and on the next request counts only the new part.
    ** Requests in ascending offset order cost O(n) in total.
    ** The "anchor" (setErrorAnchor) is used for the text of expanded entities:
    ** errors inside an entity are reported at the position of its reference in the document.
    */
    class XmlScanner
    {
    public:
        explicit XmlScanner(std::string_view input) noexcept;

        // Достигнут ли конец ввода
        // Whether the end of input is reached
        bool isAtEnd() const noexcept;

        // Текущий символ (на конце ввода — '\0')
        // The current character (at the end of input — '\0')
        char peek() const noexcept;

        // Начинается ли остаток ввода с указанного фрагмента
        // Whether the rest of the input starts with the given fragment
        bool startsWith(std::string_view fragment) const noexcept;

        // Сдвигает курсор вперёд (не дальше конца ввода)
        // Moves the cursor forward (not past the end of input)
        void advance(std::size_t count) noexcept;

        // Требует фрагмент в текущей позиции и пропускает его, иначе — ParseException
        // Requires a fragment at the current position and skips it, otherwise — ParseException
        void expect(std::string_view fragment);

        // Пропускает пробельные символы и возвращает их количество
        // Skips whitespace characters and returns their number
        std::size_t skipWhitespace() noexcept;

        // Читает XML-имя; при его отсутствии — ParseException
        // Reads an XML name; if it is missing — ParseException
        std::string_view readName();

        // Читает всё до разделителя и пропускает сам разделитель; нет разделителя — ошибка
        // Reads everything up to a delimiter and skips the delimiter itself; no delimiter — an error
        std::string_view readUntil(std::string_view delimiter);

        // Читает всё до символа (не включая его) или до конца ввода
        // Reads everything up to a character (not including it) or to the end of input
        std::string_view readUntilChar(char symbol) noexcept;

        // Читает всё до любого из символов набора (не включая его) или до конца
        // Reads everything up to any character of the set (not including it) or to the end
        std::string_view readUntilAny(std::string_view symbols) noexcept;

        // Срез входа [begin, end)
        // The input slice [begin, end)
        std::string_view slice(std::size_t begin, std::size_t end) const noexcept;

        // Текущее смещение в байтах
        // The current offset in bytes
        std::size_t getOffset() const noexcept;

        // Смещение указателя, лежащего внутри входной строки
        // The offset of a pointer lying inside the input string
        std::size_t offsetOf(const char* pointer) const noexcept;

        // Строка и столбец для смещения (столбец — в символах UTF-8)
        // The line and column for an offset (the column is in UTF-8 characters)
        ParsePosition computePosition(std::size_t offset) const noexcept;

        // Все ошибки этого сканера сообщать в позиции anchor с префиксом context
        // Report all errors of this scanner at position anchor with the prefix context
        void setErrorAnchor(ParsePosition anchor, std::string context);

        // Бросает ParseException с позицией текущего курсора
        // Throws ParseException with the position of the current cursor
        [[noreturn]] void fail(const std::string& message) const;

        // Бросает ParseException с позицией указанного смещения
        // Throws ParseException with the position of the given offset
        [[noreturn]] void failAt(const std::string& message, std::size_t offset) const;

        // То же с явным кодом ошибки (NAMESPACE_ERROR, LIMIT_EXCEEDED и т.п.)
        // The same with an explicit error code (NAMESPACE_ERROR, LIMIT_EXCEEDED and so on)
        [[noreturn]] void failAt(const std::string& message, std::size_t offset,
            DomErrorCode code) const;

    private:
        std::string_view _input;
        std::size_t _offset;
        mutable ParsePosition _cache;
        std::optional<ParsePosition> _anchor;
        std::string _context;
    };
}

#endif
