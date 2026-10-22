#include "failing/failing_list.hpp"
#include "failing/list_error.hpp"

#include <fstream>

namespace failing
{
    std::vector<FailingEntry> readFailingList(const std::filesystem::path& file)
    {
        std::ifstream in(file, std::ios::binary);
        if (!in.is_open()) {
            throw ListError("не удалось открыть список провалов " + file.u8string());
        }

        std::vector<FailingEntry> entries;
        std::string line;
        size_t lineNumber = 0;

        while (std::getline(in, line)) {
            ++lineNumber;
            // Удаление завершающего '\r'
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            // Пропуск пустых строк и строк, начинающихся с '#'
            if (line.empty() || line[0] == '#') {
                continue;
            }

            auto fields = detail::splitFields(line);
            if (fields.size() != 8) {
                throw ListError("строка " + std::to_string(lineNumber) + " списка провалов: ожидалось 8 полей");
            }

            entries.emplace_back();
            entries.back().id = fields[0];
            entries.back().kind = fields[1];
            entries.back().expected = fields[2];
            entries.back().actual = fields[3];
            entries.back().documents = fields[4];
            entries.back().schema = fields[5];
            entries.back().message = fields[6];
            entries.back().description = fields[7];
        }

        return entries;
    }

    void writeFailingList(const std::filesystem::path& file, const std::vector<FailingEntry>& entries)
    {
        std::ofstream out(file, std::ios::binary);
        if (!out.is_open()) {
            throw ListError("не удалось записать список провалов " + file.u8string());
        }

        out << "# Провалы W3C XML Schema Test Suite — создано xsts_failing --regenerate, руками не править\n";
        out << "# Failures of the W3C XML Schema Test Suite — made by xsts_failing --regenerate, do not edit by hand\n";
        out << "# id\tkind\texpected\tactual\tdocuments\tschema\tmessage\tdescription\n";

        for (const auto& entry : entries) {
            out << detail::cleanField(entry.id) << '\t'
                << detail::cleanField(entry.kind) << '\t'
                << detail::cleanField(entry.expected) << '\t'
                << detail::cleanField(entry.actual) << '\t'
                << detail::cleanField(entry.documents) << '\t'
                << detail::cleanField(entry.schema) << '\t'
                << detail::cleanField(entry.message) << '\t'
                << detail::cleanField(entry.description) << '\n';
        }
    }

    namespace detail
    {
        std::vector<std::string> splitFields(const std::string& line)
        {
            std::vector<std::string> fields(1);

            // Каждая табуляция открывает новое поле: так сохраняется и пустое последнее
            // Each tab opens a new field: this keeps an empty last field too
            for (char symbol : line) {
                if (symbol == '\t')
                    fields.emplace_back();
                else
                    fields.back() += symbol;
            }
            return fields;
        }

        std::string cleanField(const std::string& text)
        {
            std::string result;
            for (char ch : text) {
                if (ch == '\t' || ch == '\n' || ch == '\r') {
                    result += ' ';
                } else {
                    result += ch;
                }
            }
            return result;
        }

        std::string joinDocuments(const std::vector<std::filesystem::path>& documents, const std::filesystem::path& suiteRoot)
        {
            if (documents.empty()) {
                return "-";
            }

            std::vector<std::string> relativePaths;
            for (const auto& doc : documents) {
                relativePaths.push_back(doc.lexically_relative(suiteRoot).generic_u8string());
            }

            std::string result;
            for (size_t i = 0; i < relativePaths.size(); ++i) {
                result += relativePaths[i];
                if (i < relativePaths.size() - 1) {
                    result += ";";
                }
            }

            return result;
        }
    }
}
