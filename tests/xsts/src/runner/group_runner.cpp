#include "xsts/case_runner.hpp"

#include <utility>

namespace xsts
{
    GroupRunner::GroupRunner(std::filesystem::path suiteRoot, const Limitations& limitations)
        : _suiteRoot(std::move(suiteRoot)), _limitations(limitations)
    {
    }

    /*
    ** Схема загружается всегда, даже если её schemaTest пропущен как
    ** спорный: экземпляры группы проверяются по ней независимо.
    ** Вердикт схемы копируется: вектор results растёт, указатель в нём
    ** после push_back мог бы стать недействительным.
    **
    ** The schema is always loaded, even if its schemaTest is skipped as
    ** disputed: the group instances are validated against it independently.
    ** The schema verdict is copied: the results vector grows, a pointer into it
    ** could become invalid after push_back.
    */
    void GroupRunner::run(const Catalog& catalog, const TestGroup& group,
        std::vector<CaseResult>& results) const
    {
        SchemaAttempt schema;
        CaseResult schemaResult;

        if (group.hasSchema)
        {
            const TestCase& schemaCase = catalog.cases[group.schemaCase];
            schema = loadSchema(schemaCase.documents, _suiteRoot);
            schemaResult = finish(schemaCase, judgeSchema(schemaCase, schema));
            results.push_back(schemaResult);
        }
        for (std::size_t index : group.instanceCases)
        {
            const CaseResult* owner = nullptr;
            if (group.hasSchema)
                owner = &schemaResult;
            results.push_back(runInstance(catalog.cases[index], schema, owner));
        }
    }

    /*
    ** Схема группы не загрузилась:
    **  - она и не должна была (ожидалась невалидной) — экземпляр не применим;
    **  - её schemaTest — спорный или упёрся в ограничение — экземпляр
    **    наследует этот вердикт;
    **  - иначе это провал и экземпляра: проверить его не по чему.
    **
    ** The group schema did not load:
    **  - it was not supposed to (it was expected invalid) — the instance does not apply;
    **  - its schemaTest is disputed or hit a limitation — the instance
    **    inherits that verdict;
    **  - otherwise it is a failure of the instance too: there is nothing to validate it against.
    */
    CaseResult GroupRunner::runInstance(const TestCase& testCase, const SchemaAttempt& schema,
        const CaseResult* schemaResult) const
    {
        static const std::vector<std::filesystem::path> NO_DOCUMENTS;
        const std::vector<std::filesystem::path>* groupDocuments = &NO_DOCUMENTS;

        if (schemaResult != nullptr)
            groupDocuments = &schemaResult->testCase->documents;
        if (schemaResult == nullptr || schema.schema != nullptr)
            return finish(testCase, judgeInstance(testCase, validateInstance(testCase.documents.front(),
                schema.schema, *groupDocuments, _suiteRoot)));
        if (schemaResult->testCase->expected == Expectation::INVALID)
            return CaseResult{&testCase, Verdict::SKIPPED, "схема группы ожидаемо невалидна"};
        if (schemaResult->verdict == Verdict::SKIPPED || schemaResult->verdict == Verdict::LIMITED)
            return CaseResult{&testCase, schemaResult->verdict, schemaResult->reason};
        return finish(testCase, CaseResult{&testCase, Verdict::FAILED,
            "схема группы не загрузилась: " + schema.message});
    }

    // Спорные случаи пропускаются; провал случая из списка ограничений — отдельная графа
    // Disputed cases are skipped; a failure of a case from the limitation list is a separate column
    CaseResult GroupRunner::finish(const TestCase& testCase, CaseResult judged) const
    {
        std::string skip = skipReason(testCase);

        if (!skip.empty())
            return CaseResult{&testCase, Verdict::SKIPPED, skip};
        if (judged.verdict != Verdict::FAILED)
            return judged;
        std::string limitation = _limitations.reasonFor(testCase);
        if (!limitation.empty())
            return CaseResult{&testCase, Verdict::LIMITED, limitation + " — " + judged.reason};
        return judged;
    }
}
