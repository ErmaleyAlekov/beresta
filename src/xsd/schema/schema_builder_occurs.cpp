#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    // Неотрицательное целое (собственный разбор: std::stoull бросает чужие исключения)
    // A non-negative integer (an own parser: std::stoull throws foreign exceptions)
    static bool parseOccurs(const std::string& text, std::size_t& value) noexcept
    {
        value = 0;
        if (text.empty() || text.size() > 9)
            return false;
        for (char symbol : text)
        {
            if (symbol < '0' || symbol > '9')
                return false;
            value = value * 10 + static_cast<std::size_t>(symbol - '0');
        }
        return true;
    }

    // minOccurs (по умолчанию 1) и maxOccurs (число или unbounded)
    // minOccurs (1 by default) and maxOccurs (a number or unbounded)
    void SchemaBuilder::readOccurs(const Element& definition, content::Particle& particle) const
    {
        std::string minimum = attributeOr(definition, "minOccurs", "1");
        std::string maximum = attributeOr(definition, "maxOccurs", "1");

        if (!parseOccurs(minimum, particle.minOccurs))
            fail(definition, i18n::format(i18n::Message::SCHEMA_MIN_OCCURS, {minimum}));
        particle.maxOccurs = content::UNBOUNDED;
        if (maximum != "unbounded" && !parseOccurs(maximum, particle.maxOccurs))
            fail(definition, i18n::format(i18n::Message::SCHEMA_MAX_OCCURS, {maximum}));
        if (particle.maxOccurs < particle.minOccurs)
            fail(definition, i18n::format(i18n::Message::SCHEMA_MAX_BELOW_MIN));
    }
}
