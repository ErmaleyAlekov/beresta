#include "xsts/report.hpp"

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

namespace xsts
{
    namespace
    {
        // Сообщения библиотеки бывают многострочными — в списке Markdown нужна одна строка
        // Library messages can be multi-line — a Markdown list needs one line
        std::string oneLine(const std::string& text)
        {
            const std::size_t LIMIT = 400;
            std::string line;

            for (char symbol : text)
            {
                if (symbol == '\n' || symbol == '\r' || symbol == '|')
                    symbol = ' ';
                line += symbol;
            }
            if (line.size() > LIMIT)
                line = line.substr(0, LIMIT) + "…";
            return line;
        }

        /*
        ** Строка расхождения: ID, категория, документ, описание группы,
        ** причина и метки возможностей (без повторов: метки экземпляра
        ** и его схемы могут совпадать).
        **
        ** A deviation line: the ID, the category, the document, the group description,
        ** the reason and the feature labels (without repeats: the labels of an instance
        ** and its schema may coincide).
        */
        void writeFailureLine(const CaseResult& result, std::ostream& out)
        {
            const TestCase& testCase = *result.testCase;
            std::set<std::string> features(testCase.features.begin(), testCase.features.end());
            std::string labels;

            for (const std::string& feature : features)
            {
                if (!labels.empty())
                    labels += ", ";
                labels += feature;
            }
            out << "- `" << testCase.id << "` (" << kindName(testCase) << "; " << testCase.location
                << ") «" << testCase.description << "» — " << oneLine(result.reason);
            if (!labels.empty())
                out << " [" << labels << "]";
            out << "\n";
        }
    }

    namespace detail
    {
        /*
        ** Расхождения по наборам: ID, категория, документ, причина.
        ** Именно этот список разбирается после прогона: исправление в
        ** библиотеке или запись в known_limitations.txt.
        **
        ** The deviations by sets: the ID, the category, the document, the reason.
        ** This is the list analysed after the run: a fix in
        ** the library or an entry in known_limitations.txt.
        */
        void writeFailures(const std::vector<CaseResult>& results, std::ostream& out)
        {
            std::map<std::string, std::vector<const CaseResult*>> bySet;

            for (const CaseResult& result : results)
            {
                if (result.verdict == Verdict::FAILED)
                    bySet[result.testCase->testSet].push_back(&result);
            }
            out << "\n## Расхождения\n";
            if (bySet.empty())
                out << "\nРасхождений нет.\n";
            for (const auto& entry : bySet)
            {
                out << "\n### " << entry.first << " (" << entry.second.size() << ")\n\n";
                for (const CaseResult* result : entry.second)
                    writeFailureLine(*result, out);
            }
        }

        /*
        ** Провалы из-за сознательных ограничений: причина (часть до « — »)
        ** и список случаев. Процент их не учитывает, но скрывать их нельзя.
        **
        ** Failures caused by deliberate limitations: the reason (the part before " — ")
        ** and the list of cases. The percentage does not count them, but they must not be hidden.
        */
        void writeLimited(const std::vector<CaseResult>& results, std::ostream& out)
        {
            std::map<std::string, std::vector<std::string>> byReason;

            for (const CaseResult& result : results)
            {
                if (result.verdict == Verdict::LIMITED)
                    byReason[result.reason.substr(0, result.reason.find(" — "))].push_back(result.testCase->id);
            }
            out << "\n## Сознательные ограничения\n\n";
            if (byReason.empty())
                out << "Случаев нет.\n";
            for (const auto& entry : byReason)
            {
                out << "- **" << entry.first << "** — " << entry.second.size() << ":";
                for (const std::string& id : entry.second)
                    out << " `" << id << "`";
                out << "\n";
            }
        }
    }
}
