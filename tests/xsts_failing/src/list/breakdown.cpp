#include "failing/regenerate.hpp"

#include <iomanip>
#include <map>
#include <ostream>
#include <string>

namespace failing
{
    namespace
    {
        // Имя набора — первая часть id до "/"
        // The set name is the first part of the id before "/"
        std::string setOf(const std::string& id)
        {
            return id.substr(0, id.find('/'));
        }

        // Строки «число  название»: число впереди, чтобы кириллица не сбивала колонки
        // "count  name" lines: the count goes first so that Cyrillic does not break the columns
        void printCounts(const std::map<std::string, std::size_t>& counts, std::ostream& out)
        {
            for (const auto& entry : counts)
                out << "    " << std::setw(5) << entry.second << "  " << entry.first << "\n";
        }

        // Строка набора: всего и раскладка по видам случаев
        // A set line: the total and the split by case kinds
        void printSet(const std::string& name, const std::map<std::string, std::size_t>& kinds,
            std::ostream& out)
        {
            std::size_t total = 0;
            std::string split;

            for (const auto& entry : kinds)
            {
                total += entry.second;
                if (!split.empty())
                    split += ", ";
                split += entry.first + ": " + std::to_string(entry.second);
            }
            out << "    " << std::setw(5) << total << "  " << name << " (" << split << ")\n";
        }
    }

    /*
    ** Две таблицы: «вид ожидание -> факт» (какого рода ошибка) и наборы
    ** (где она живёт). Обе нужны README, чтобы выбрать, с чего начать.
    **
    ** Two tables: "kind expectation -> fact" (what sort of error it is) and the sets
    ** (where it lives). The README needs both to choose where to start.
    */
    void printBreakdown(const std::vector<FailingEntry>& entries, std::ostream& out)
    {
        std::map<std::string, std::map<std::string, std::size_t>> bySet;
        std::map<std::string, std::size_t> byKind;

        for (const FailingEntry& entry : entries)
        {
            ++bySet[setOf(entry.id)][entry.kind + " " + entry.expected];
            ++byKind[entry.kind + " " + entry.expected + " -> " + entry.actual];
        }
        out << "  По видам (ожидание -> факт):\n";
        printCounts(byKind, out);
        out << "\n  По наборам:\n";
        for (const auto& entry : bySet)
            printSet(entry.first, entry.second, out);
    }
}
