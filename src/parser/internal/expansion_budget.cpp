#include "parser/internal/expansion_budget.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

namespace beresta::detail
{
    // Лимиты берутся из настроек парсера
    // The limits are taken from the parser settings
    ExpansionBudget::ExpansionBudget(const ParserOptions& options) noexcept
        : _maxExpansions(options.maxEntityExpansions), _maxBytes(options.maxExpandedSize),
          _maxDepth(options.maxEntityDepth), _expansions(0), _bytes(0)
    {
    }

    // Три независимых лимита: число раскрытий, суммарный объём и вложенность
    // Three independent limits: the number of expansions, the total size and the nesting
    void ExpansionBudget::charge(std::size_t bytes, std::size_t depth)
    {
        ++_expansions;
        _bytes += bytes;
        if (_expansions > _maxExpansions)
            throw DomException(DomErrorCode::LIMIT_EXCEEDED,
                i18n::format(i18n::Message::LIMIT_EXPANSIONS, {std::to_string(_maxExpansions)}));
        if (_bytes > _maxBytes)
            throw DomException(DomErrorCode::LIMIT_EXCEEDED,
                i18n::format(i18n::Message::LIMIT_EXPANDED_SIZE, {std::to_string(_maxBytes)}));
        if (depth > _maxDepth)
            throw DomException(DomErrorCode::LIMIT_EXCEEDED,
                i18n::format(i18n::Message::LIMIT_ENTITY_DEPTH, {std::to_string(_maxDepth)}));
    }
}
