#ifndef XSD_VALIDATION_BUDGET_HPP
#define XSD_VALIDATION_BUDGET_HPP

#include <cstddef>
#include <string_view>

namespace beresta::xsd::detail
{
    // Бюджет каждой коллекции: объём данных и стоимость записей.
    // Per-collection budget for payload bytes and record storage.
    constexpr std::size_t VALIDATION_BUDGET = 64 * 1024 * 1024;
    void chargeValidation(std::size_t& used, std::size_t amount, std::string_view resource);
}

#endif
