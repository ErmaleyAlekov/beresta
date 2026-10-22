#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Неотрицательное целое (собственный разбор: std::stoull бросает чужие
    ** исключения). minOccurs / maxOccurs — любое xs:nonNegativeInteger,
    ** поэтому слишком большое число не ошибка: оно насыщается до
    ** наибольшего КОНЕЧНОГО значения OCCURS_LIMIT. UNBOUNDED занят под
    ** maxOccurs="unbounded", и конечная граница не должна с ним совпасть.
    ** Столько вхождений в документе всё равно не бывает, а автомат считает
    ** вхождения счётчиком и частицу не разворачивает.
    **
    ** A non-negative integer (an own parser: std::stoull throws foreign
    ** exceptions). minOccurs / maxOccurs is any xs:nonNegativeInteger,
    ** so a too large number is not an error: it saturates to the
    ** largest FINITE value OCCURS_LIMIT. UNBOUNDED is taken by
    ** maxOccurs="unbounded", and a finite bound must not coincide with it.
    ** A document never has that many occurrences anyway, and the automaton counts
    ** occurrences with a counter and does not unroll the particle.
    */
    static bool parseOccurs(const std::string& text, std::size_t& value) noexcept
    {
        const std::size_t OCCURS_LIMIT = content::UNBOUNDED - 1;

        value = 0;
        if (text.empty())
            return false;
        for (char symbol : text)
        {
            if (symbol < '0' || symbol > '9')
                return false;
            std::size_t digit = static_cast<std::size_t>(symbol - '0');
            if (value > (OCCURS_LIMIT - digit) / 10)
                value = OCCURS_LIMIT;
            else
                value = value * 10 + digit;
        }
        return true;
    }

    /*
    ** minOccurs (по умолчанию 1) и maxOccurs (число или unbounded).
    ** У самой группы xs:all обе границы — только 0 или 1 (3.8.6, all-group).
    **
    ** minOccurs (1 by default) and maxOccurs (a number or unbounded).
    ** For the xs:all group itself both bounds are only 0 or 1 (3.8.6, all-group).
    */
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
        if (isSchemaElement(definition, "all") && particle.maxOccurs > 1)
            fail(definition, i18n::format(i18n::Message::SCHEMA_ALL_OCCURS));
    }
}
