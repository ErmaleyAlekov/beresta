#include "xsd/internal/schema_builder.hpp"

namespace beresta::xsd::detail
{
    // Неотрицательное целое (собственный разбор: std::stoull бросает чужие исключения)
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
    void SchemaBuilder::readOccurs(const Element& definition, content::Particle& particle) const
    {
        std::string minimum = attributeOr(definition, "minOccurs", "1");
        std::string maximum = attributeOr(definition, "maxOccurs", "1");

        if (!parseOccurs(minimum, particle.minOccurs))
            fail(definition, "некорректный minOccurs \"" + minimum + "\"");
        particle.maxOccurs = content::UNBOUNDED;
        if (maximum != "unbounded" && !parseOccurs(maximum, particle.maxOccurs))
            fail(definition, "некорректный maxOccurs \"" + maximum + "\"");
        if (particle.maxOccurs < particle.minOccurs)
            fail(definition, "maxOccurs меньше minOccurs");
    }
}
