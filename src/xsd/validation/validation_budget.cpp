#include "xsd/internal/validation_budget.hpp"
#include "exception/dom_exception.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    void chargeValidation(std::size_t& used, std::size_t amount, std::string_view resource)
    {
        if (amount > VALIDATION_BUDGET - used)
            throw DomException(DomErrorCode::SCHEMA_ERROR, i18n::format(i18n::Message::XSD_RESOURCE_LIMIT, {resource}));
        used += amount;
    }
}
