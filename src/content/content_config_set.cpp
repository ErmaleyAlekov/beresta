#include "content/content_cursor.hpp"

namespace beresta::content
{
    // До стольких конфигураций повторы ищутся перебором, дальше — через множество
    // Up to this many configurations repetitions are found by a loop, beyond that — through a set
    static const std::size_t FEW_CONFIGS = 16;

    // Очистка без освобождения памяти: объекты конфигураций остаются для следующего шага
    // Clearing without freeing memory: the configuration objects stay for the next step
    void ConfigSet::clear()
    {
        count = 0;
        seen.clear();
    }

    /*
    ** Добавление без повторов. Почти всегда конфигураций одна-две (модели
    ** XSD обязаны быть однозначными), и перебор дешевле всего; для
    ** неоднозначных моделей с сотнями конфигураций включается множество,
    ** чтобы шаг не стал квадратичным.
    **
    ** Adding without repetitions. Almost always there are one or two configurations (XSD
    ** models must be unambiguous), and a loop is the cheapest; for
    ** ambiguous models with hundreds of configurations the set is switched on
    ** so that a step does not become quadratic.
    */
    void ConfigSet::add(const MatchConfig& config)
    {
        if (count < FEW_CONFIGS)
        {
            for (std::size_t index = 0; index < count; ++index)
            {
                if (items[index] == config)
                    return;
            }
        }
        else
        {
            if (seen.size() != count)
                seen.insert(items.begin(), items.begin() + static_cast<std::ptrdiff_t>(count));
            if (!seen.insert(config).second)
                return;
        }
        if (count < items.size())
            items[count] = config;
        else
            items.push_back(config);
        ++count;
    }
}
