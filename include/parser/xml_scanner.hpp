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
    */
    class XmlScanner
    {
    public:
        explicit XmlScanner(std::string_view input) noexcept;

        // Достигнут ли конец ввода
        bool isAtEnd() const noexcept;

        // Текущий символ (на конце ввода — '\0')
        char peek() const noexcept;

        // Начинается ли остаток ввода с указанного фрагмента
        bool startsWith(std::string_view fragment) const noexcept;

        // Сдвигает курсор вперёд (не дальше конца ввода)
        void advance(std::size_t count) noexcept;

        // Требует фрагмент в текущей позиции и пропускает его, иначе — ParseException
        void expect(std::string_view fragment);

        // Пропускает пробельные символы и возвращает их количество
        std::size_t skipWhitespace() noexcept;

        // Читает XML-имя; при его отсутствии — ParseException
        std::string_view readName();

        // Читает всё до разделителя и пропускает сам разделитель; нет разделителя — ошибка
        std::string_view readUntil(std::string_view delimiter);

        // Читает всё до символа (не включая его) или до конца ввода
        std::string_view readUntilChar(char symbol) noexcept;

        // Читает всё до любого из символов набора (не включая его) или до конца
        std::string_view readUntilAny(std::string_view symbols) noexcept;

        // Срез входа [begin, end)
        std::string_view slice(std::size_t begin, std::size_t end) const noexcept;

        // Текущее смещение в байтах
        std::size_t getOffset() const noexcept;

        // Смещение указателя, лежащего внутри входной строки
        std::size_t offsetOf(const char* pointer) const noexcept;

        // Строка и столбец для смещения (столбец — в символах UTF-8)
        ParsePosition computePosition(std::size_t offset) const noexcept;

        // Все ошибки этого сканера сообщать в позиции anchor с префиксом context
        void setErrorAnchor(ParsePosition anchor, std::string context);

        // Бросает ParseException с позицией текущего курсора
        [[noreturn]] void fail(const std::string& message) const;

        // Бросает ParseException с позицией указанного смещения
        [[noreturn]] void failAt(const std::string& message, std::size_t offset) const;

        // То же с явным кодом ошибки (NAMESPACE_ERROR, LIMIT_EXCEEDED и т.п.)
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
