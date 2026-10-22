#include "failing/debug_run.hpp"
#include "failing/list_error.hpp"

#include <iomanip>
#include <ostream>

namespace failing
{
    /*
    ** Случаи выполняются строго по одному и печатаются сразу (std::endl):
    ** если библиотека упадёт, последний напечатанный номер подскажет,
    ** на каком случае это произошло.
    **
    ** The cases are run strictly one by one and printed at once (std::endl):
    ** if the library crashes, the last printed number tells
    ** which case it happened on.
    */
    RunTotals runListed(const std::vector<FailingEntry>& entries, const SuiteView& suite,
        const RunOptions& options, std::ostream& out)
    {
        RunTotals totals;

        for (std::size_t position = 0; position < entries.size(); ++position)
        {
            const FailingEntry& entry = entries[position];

            if (!detail::isSelected(entry, position + 1, options))
                continue;
            Execution execution = executeCase(suite, detail::caseIndexOf(suite, entry.id));
            ++totals.selected;
            if (execution.passed)
                ++totals.passed;
            detail::printCase(position + 1, entry, execution, out);
            if (options.stopOnFirst && !execution.passed)
                break;
        }
        return totals;
    }

    namespace detail
    {
        // Номер (если задан) должен совпасть, id — начинаться с префикса
        // The number (if given) must match, the id must start with the prefix
        bool isSelected(const FailingEntry& entry, std::size_t number, const RunOptions& options)
        {
            if (options.caseNumber != 0 && options.caseNumber != number)
                return false;
            return entry.id.compare(0, options.prefix.size(), options.prefix) == 0;
        }

        // Список мог устареть относительно набора — это ошибка окружения, а не вердикт
        // The list may be out of date with the suite — an environment error, not a verdict
        std::size_t caseIndexOf(const SuiteView& suite, const std::string& id)
        {
            auto found = suite.index.byId.find(id);

            if (found == suite.index.byId.end())
                throw ListError("случая " + id + " нет в наборе — пересоберите список (make regen)");
            return found->second;
        }

        // Блок из 5–6 строк: заголовок, описание, документы, схема группы, ожидание/факт, сообщение
        // A block of 5–6 lines: the header, the description, the documents, the group schema,
        // expectation/fact, the message
        void printCase(std::size_t number, const FailingEntry& entry, const Execution& execution,
            std::ostream& out)
        {
            out << "[" << std::setw(4) << number << "] " << entry.kind << "  " << entry.id << "\n";
            out << "       описание:  " << entry.description << "\n";
            out << "       документы: " << entry.documents << "\n";
            if (entry.schema != "-")
                out << "       схема:     " << entry.schema << "\n";
            out << "       ожидалось: " << entry.expected << ", получено: " << execution.actual;
            if (execution.passed)
                out << " — ИСПРАВЛЕН\n";
            else
                out << " — провал\n";
            out << "       сообщение: " << execution.message << std::endl;
        }
    }
}
