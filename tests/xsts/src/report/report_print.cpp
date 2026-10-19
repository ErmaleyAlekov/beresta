#include "xsts/report.hpp"

#include <ostream>
#include <string>

namespace xsts
{
    namespace
    {
        /*
        ** Ячейка консольной таблицы ширины width. std::setw считает байты,
        ** а кириллица в UTF-8 занимает по два байта на букву, поэтому
        ** ширина считается в символах: байты продолжения (10xxxxxx) не в счёт.
        ** alignLeft — выравнивание по левому краю (имена), иначе по правому (числа).
        **
        ** A console table cell of width width. std::setw counts bytes,
        ** and Cyrillic in UTF-8 takes two bytes per letter, so
        ** the width is counted in characters: continuation bytes (10xxxxxx) do not count.
        ** alignLeft — left alignment (names), otherwise right (numbers).
        */
        std::string cell(const std::string& text, std::size_t width, bool alignLeft)
        {
            std::size_t symbols = 0;

            for (char byte : text)
            {
                if ((static_cast<unsigned char>(byte) & 0xC0) != 0x80)
                    ++symbols;
            }
            if (symbols >= width)
                return text;
            std::string padding(width - symbols, ' ');
            if (alignLeft)
                return text + padding;
            return padding + text;
        }

        // Одна строка консольной таблицы: имя и четыре счётчика с процентом
        // One row of the console table: the name and four counters with the percentage
        void printRow(const std::string& name, const std::string (&values)[5], std::ostream& out)
        {
            out << "  " << cell(name, 24, true);
            for (const std::string& value : values)
                out << cell(value, 10, false);
            out << "\n";
        }

        // Счётчики группы как строки ячеек: прошло, провал, ограничение, пропуск, процент
        // The group counters as cell strings: passed, failed, limited, skipped, percentage
        void printTally(const std::string& name, const Tally& tally, std::ostream& out)
        {
            const std::string values[5] = {std::to_string(tally.passed), std::to_string(tally.failed),
                std::to_string(tally.limited), std::to_string(tally.skipped), detail::percent(tally)};

            printRow(name, values, out);
        }
    }

    // Краткая сводка по категориям — то, что видно сразу после make run
    // A short summary by categories — what is visible right after make run
    void printSummary(const Summary& summary, std::ostream& out)
    {
        const std::string header[5] = {"прошло", "провал", "огранич.", "пропуск", "процент"};

        out << "\n";
        printRow("Категория", header, out);
        for (const auto& entry : summary.byKind)
            printTally(entry.first, entry.second, out);
        printTally("Итого", summary.total, out);
    }
}
