#ifndef INPUT_STACK_HPP
#define INPUT_STACK_HPP

#include "parser/xml_scanner.hpp"

#include <cstddef>
#include <deque>
#include <string_view>

namespace beresta::detail
{
    /*
    ** Источник ввода: основной документ или текст раскрываемой сущности.
    ** openElements — сколько элементов было открыто в момент раскрытия:
    ** по стандарту сущность обязана закрыть все открытые ею элементы.
    ** mainOffset — смещение ссылки в основном документе (для позиций узлов).
    */
    struct InputFrame
    {
        XmlScanner scanner;
        std::string_view entityName;
        std::size_t openElements;
        std::size_t mainOffset;
    };

    /*
    ** InputStack — стек источников ввода. Ссылка &name; на сущность с
    ** разметкой кладёт её текст на вершину, и парсер продолжает разбор
    ** уже из него — так содержимое сущности разбирается как обычный XML.
    ** std::deque не инвалидирует ссылки на элементы при добавлении.
    */
    class InputStack
    {
    public:
        explicit InputStack(std::string_view mainText);

        // Текущий источник и основной документ
        XmlScanner& current() noexcept;
        XmlScanner& main() noexcept;
        const XmlScanner& main() const noexcept;

        // Вершина стека
        const InputFrame& top() const noexcept;

        // Идёт ли сейчас разбор текста сущности
        bool inEntity() const noexcept;

        // Количество раскрытых сущностей
        std::size_t depth() const noexcept;

        // Раскрывается ли уже сущность name (защита от рекурсии)
        bool isExpanding(std::string_view name) const noexcept;

        // Кладёт текст сущности; ошибки внутри будут сообщаться в позиции ссылки
        void push(std::string_view name, std::string_view text, std::size_t openElements,
            std::size_t mainOffset);

        // Снимает исчерпанную сущность
        void pop() noexcept;

        // Смещение в основном документе для позиции offset текущего источника
        std::size_t mainOffsetFor(std::size_t offset) const noexcept;

    private:
        std::deque<InputFrame> _frames;
    };
}

#endif
