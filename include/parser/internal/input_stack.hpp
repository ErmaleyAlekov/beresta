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
    **
    ** An input source: the main document or the text of an entity being expanded.
    ** openElements — how many elements were open at the moment of expansion:
    ** by the standard an entity must close all elements it opened.
    ** mainOffset — the offset of the reference in the main document (for node positions).
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
    **
    ** InputStack — a stack of input sources. A reference &name; to an entity with
    ** markup pushes its text on top, and the parser continues parsing
    ** from it — so the entity content is parsed as ordinary XML.
    ** std::deque does not invalidate references to elements on insertion.
    */
    class InputStack
    {
    public:
        explicit InputStack(std::string_view mainText);

        // Текущий источник и основной документ
        // The current source and the main document
        XmlScanner& current() noexcept;
        XmlScanner& main() noexcept;
        const XmlScanner& main() const noexcept;

        // Вершина стека
        // The top of the stack
        const InputFrame& top() const noexcept;

        // Идёт ли сейчас разбор текста сущности
        // Whether an entity text is being parsed now
        bool inEntity() const noexcept;

        // Количество раскрытых сущностей
        // The number of expanded entities
        std::size_t depth() const noexcept;

        // Раскрывается ли уже сущность name (защита от рекурсии)
        // Whether the entity name is already being expanded (protection against recursion)
        bool isExpanding(std::string_view name) const noexcept;

        // Кладёт текст сущности; ошибки внутри будут сообщаться в позиции ссылки
        // Pushes an entity text; errors inside will be reported at the reference position
        void push(std::string_view name, std::string_view text, std::size_t openElements,
            std::size_t mainOffset);

        // Снимает исчерпанную сущность
        // Pops an exhausted entity
        void pop() noexcept;

        // Смещение в основном документе для позиции offset текущего источника
        // The offset in the main document for position offset of the current source
        std::size_t mainOffsetFor(std::size_t offset) const noexcept;

    private:
        std::deque<InputFrame> _frames;
    };
}

#endif
