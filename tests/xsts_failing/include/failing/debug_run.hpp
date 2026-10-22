#ifndef FAILING_DEBUG_RUN_HPP
#define FAILING_DEBUG_RUN_HPP

#include "failing/case_executor.hpp"
#include "failing/failing_case.hpp"

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

namespace failing
{
    // Какие случаи списка выполнять и когда остановиться
    // Which list cases to run and when to stop
    struct RunOptions
    {
        std::string prefix;              // префикс id ("MS-Notations", "MGroup/mgO") / an id prefix
        std::size_t caseNumber = 0;      // номер записи в списке (0 — все) / the entry number in the list (0 — all)
        bool stopOnFirst = false;        // остановиться на первом не исправленном случае
                                         // stop at the first case that is not fixed
    };

    // Итог прогона списка: сколько выбрано и сколько уже проходит
    // The outcome of running the list: how many were selected and how many already pass
    struct RunTotals
    {
        std::size_t selected = 0;
        std::size_t passed = 0;
    };

    /*
    ** Выполняет выбранные случаи списка по одному (паттерн Iterator над
    ** записями с фильтром) и печатает по каждому ожидание, факт и сообщение.
    **
    ** Runs the selected list cases one by one (the Iterator pattern over
    ** the entries with a filter) and prints the expectation, the fact and the message for each.
    */
    RunTotals runListed(const std::vector<FailingEntry>& entries, const SuiteView& suite,
        const RunOptions& options, std::ostream& out);

    namespace detail
    {
        // Подходит ли запись под фильтр (номер считается с 1)
        // Whether an entry matches the filter (the number counts from 1)
        bool isSelected(const FailingEntry& entry, std::size_t number, const RunOptions& options);

        // Индекс случая в каталоге по id; нет в наборе — failing::ListError
        // The case index in the catalog by id; absent from the suite — failing::ListError
        std::size_t caseIndexOf(const SuiteView& suite, const std::string& id);

        // Печатает один случай: номер, id, документы, ожидание, факт и сообщение
        // Prints one case: the number, the id, the documents, the expectation, the fact and the message
        void printCase(std::size_t number, const FailingEntry& entry, const Execution& execution,
            std::ostream& out);
    }
}

#endif
