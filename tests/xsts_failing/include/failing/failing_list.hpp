#ifndef FAILING_FAILING_LIST_HPP
#define FAILING_FAILING_LIST_HPP

#include "failing/failing_case.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace failing
{
    /*
    ** Формат failing_cases.tsv: UTF-8, строки «# ...» — комментарии
    ** (шапка с названиями колонок), пустые строки пропускаются,
    ** остальные — 8 полей через табуляцию в порядке полей FailingEntry:
    **     id  kind  expected  actual  documents  schema  message  description
    ** Номер случая для --case — порядковый номер записи (с 1).
    **
    ** The failing_cases.tsv format: UTF-8, "# ..." lines are comments
    ** (a header with the column names), empty lines are skipped,
    ** the others are 8 tab-separated fields in the order of the FailingEntry fields:
    **     id  kind  expected  actual  documents  schema  message  description
    ** The case number for --case is the ordinal number of the entry (from 1).
    */

    // Читает список; нет файла или строка не из 8 полей — failing::ListError
    // Reads the list; no file or a line without 8 fields — failing::ListError
    std::vector<FailingEntry> readFailingList(const std::filesystem::path& file);

    // Пишет список с шапкой-комментарием; не открылся файл — failing::ListError
    // Writes the list with a comment header; the file did not open — failing::ListError
    void writeFailingList(const std::filesystem::path& file, const std::vector<FailingEntry>& entries);

    namespace detail
    {
        // Делит строку по табуляциям (пустые поля сохраняются)
        // Splits a line by tabs (empty fields are kept)
        std::vector<std::string> splitFields(const std::string& line);

        // Делает текст одним полем: табуляции и переводы строк -> пробелы
        // Makes a text one field: tabs and line breaks -> spaces
        std::string cleanField(const std::string& text);

        // Пути относительно корня набора через ";" (прямые слеши, UTF-8)
        // Paths relative to the suite root separated by ";" (forward slashes, UTF-8)
        std::string joinDocuments(const std::vector<std::filesystem::path>& documents,
            const std::filesystem::path& suiteRoot);
    }
}

#endif
