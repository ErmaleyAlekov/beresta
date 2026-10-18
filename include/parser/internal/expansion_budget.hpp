#ifndef EXPANSION_BUDGET_HPP
#define EXPANSION_BUDGET_HPP

#include "parser/parser_options.hpp"

#include <cstddef>

namespace beresta::detail
{
    /*
    ** ExpansionBudget — защита от атак через сущности («billion laughs»):
    ** документ размером в килобайт может объявить сущности, раскрывающиеся
    ** в гигабайты. Каждое раскрытие «оплачивается» из бюджета; превышение
    ** любого лимита — DomException(LIMIT_EXCEEDED). Аналог SecurityManager
    ** в Xerces, но включён всегда, а не по отдельной настройке.
    **
    ** ExpansionBudget — protection against entity attacks ("billion laughs"):
    ** a kilobyte document can declare entities that expand into gigabytes.
    ** Every expansion is "paid" from the budget; exceeding any limit is
    ** DomException(LIMIT_EXCEEDED). The analog of SecurityManager
    ** in Xerces, but always enabled rather than by a separate setting.
    */
    class ExpansionBudget
    {
    public:
        explicit ExpansionBudget(const ParserOptions& options) noexcept;

        // Учитывает одно раскрытие текста длиной bytes на глубине depth
        // Accounts for one expansion of a text of bytes length at depth depth
        void charge(std::size_t bytes, std::size_t depth);

    private:
        std::size_t _maxExpansions;
        std::size_t _maxBytes;
        std::size_t _maxDepth;
        std::size_t _expansions;
        std::size_t _bytes;
    };
}

#endif
