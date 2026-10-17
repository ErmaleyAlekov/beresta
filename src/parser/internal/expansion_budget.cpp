#include "parser/internal/expansion_budget.hpp"
#include "exception/dom_exception.hpp"

namespace beresta::detail
{
    // Лимиты берутся из настроек парсера
    ExpansionBudget::ExpansionBudget(const ParserOptions& options) noexcept
        : _maxExpansions(options.maxEntityExpansions), _maxBytes(options.maxExpandedSize),
          _maxDepth(options.maxEntityDepth), _expansions(0), _bytes(0)
    {
    }

    // Три независимых лимита: число раскрытий, суммарный объём и вложенность
    void ExpansionBudget::charge(std::size_t bytes, std::size_t depth)
    {
        ++_expansions;
        _bytes += bytes;
        if (_expansions > _maxExpansions)
            throw DomException(DomErrorCode::LIMIT_EXCEEDED,
                "превышено допустимое число раскрытий сущностей ("
                + std::to_string(_maxExpansions) + ")");
        if (_bytes > _maxBytes)
            throw DomException(DomErrorCode::LIMIT_EXCEEDED,
                "превышен допустимый объём текста из сущностей ("
                + std::to_string(_maxBytes) + " байт)");
        if (depth > _maxDepth)
            throw DomException(DomErrorCode::LIMIT_EXCEEDED,
                "превышена допустимая вложенность сущностей (" + std::to_string(_maxDepth) + ")");
    }
}
