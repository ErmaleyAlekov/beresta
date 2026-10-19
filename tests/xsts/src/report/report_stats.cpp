#include "xsts/report.hpp"

#include <cstdio>
#include <set>
#include <string>

namespace xsts
{
    namespace
    {
        // Добавляет вердикт в счётчики группы
        // Adds a verdict to the group counters
        void count(Tally& tally, Verdict verdict)
        {
            if (verdict == Verdict::PASSED)
                ++tally.passed;
            else if (verdict == Verdict::FAILED)
                ++tally.failed;
            else if (verdict == Verdict::LIMITED)
                ++tally.limited;
            else
                ++tally.skipped;
        }
    }

    /*
    ** Каждый случай попадает в свою категорию (вид + ожидание), в свой набор,
    ** в каталог данных и во все строки своих меток возможностей (метки
    ** экземпляра и его схемы могут повторяться — повтор считается один раз).
    **
    ** Each case goes into its category (kind + expectation), its set,
    ** its data directory and into all rows of its feature labels (the labels
    ** of an instance and its schema may repeat — a repeat counts once).
    */
    Summary summarize(const std::vector<CaseResult>& results)
    {
        Summary summary;

        for (const CaseResult& result : results)
        {
            const TestCase& testCase = *result.testCase;
            std::set<std::string> features(testCase.features.begin(), testCase.features.end());

            count(summary.byKind[kindName(testCase)], result.verdict);
            count(summary.byTestSet[testCase.testSet], result.verdict);
            count(summary.byDirectory[testCase.directory], result.verdict);
            for (const std::string& feature : features)
                count(summary.byFeature[feature], result.verdict);
            count(summary.total, result.verdict);
        }
        return summary;
    }

    // Категория — вид случая и ожидаемый результат
    // The category is the case kind and the expected result
    std::string kindName(const TestCase& testCase)
    {
        std::string name = "instanceTest";

        if (testCase.kind == CaseKind::SCHEMA)
            name = "schemaTest";
        if (testCase.expected == Expectation::VALID)
            return name + " valid";
        return name + " invalid";
    }

    namespace detail
    {
        // Знаменатель — применимые случаи: прошедшие и проваленные
        // The denominator is the applicable cases: passed and failed
        std::string percent(const Tally& tally)
        {
            std::size_t applicable = tally.passed + tally.failed;
            char buffer[32];

            if (applicable == 0)
                return "—";
            std::snprintf(buffer, sizeof(buffer), "%.1f%%",
                100.0 * static_cast<double>(tally.passed) / static_cast<double>(applicable));
            return buffer;
        }
    }
}
