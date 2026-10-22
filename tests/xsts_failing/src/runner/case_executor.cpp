#include "failing/case_executor.hpp"

#include <vector>

namespace failing
{
    /*
    ** Ставьте точку останова здесь и запускайте с --case N: выполняется
    ** ровно один случай. Для instanceTest сначала грузится схема группы,
    ** потом проверяется экземпляр — точка внутри библиотеки может
    ** сработать на обоих шагах.
    **
    ** Put a breakpoint here and run with --case N: exactly one case
    ** is run. For an instanceTest the group schema is loaded first,
    ** then the instance is validated — a breakpoint inside the library may
    ** fire on both steps.
    */
    Execution executeCase(const SuiteView& suite, std::size_t caseIndex)
    {
        const xsts::TestCase& testCase = suite.catalog.cases[caseIndex];

        if (testCase.kind == xsts::CaseKind::SCHEMA)
            return detail::executeSchema(testCase, suite.root);
        const xsts::TestGroup& group = suite.catalog.groups[suite.index.groupOf[caseIndex]];
        return detail::executeInstance(suite, group, testCase);
    }

    namespace detail
    {
        // Загрузка схемы и вердикт — те же функции, что у прогонщика tests/xsts
        // Loading the schema and the verdict — the same functions as in the tests/xsts runner
        Execution executeSchema(const xsts::TestCase& testCase, const std::filesystem::path& root)
        {
            xsts::SchemaAttempt attempt = xsts::loadSchema(testCase.documents, root);
            xsts::CaseResult judged = xsts::judgeSchema(testCase, attempt);

            return Execution{judged.verdict == xsts::Verdict::PASSED, schemaActual(attempt), judged.reason};
        }

        /*
        ** Повторяет GroupRunner::runInstance для одного экземпляра: группа
        ** со схемой — экземпляр проверяется по ней (не загрузилась — проверять
        ** не по чему), группа без схемы — схема из подсказок экземпляра.
        **
        ** Repeats GroupRunner::runInstance for one instance: a group
        ** with a schema — the instance is validated against it (it did not load — there is
        ** nothing to validate against), a group without a schema — the schema from the instance hints.
        */
        Execution executeInstance(const SuiteView& suite, const xsts::TestGroup& group,
            const xsts::TestCase& testCase)
        {
            static const std::vector<std::filesystem::path> NO_DOCUMENTS;
            const std::vector<std::filesystem::path>* groupDocuments = &NO_DOCUMENTS;
            xsts::SchemaAttempt schema;

            if (group.hasSchema)
            {
                groupDocuments = &suite.catalog.cases[group.schemaCase].documents;
                schema = xsts::loadSchema(*groupDocuments, suite.root);
                if (schema.schema == nullptr)
                    return Execution{false, "нет схемы", "схема группы не загрузилась: " + schema.message};
            }
            xsts::InstanceAttempt attempt = xsts::validateInstance(testCase.documents.front(),
                schema.schema, *groupDocuments, suite.root);
            xsts::CaseResult judged = xsts::judgeInstance(testCase, attempt);
            return Execution{judged.verdict == xsts::Verdict::PASSED, instanceActual(attempt), judged.reason};
        }

        // Схема отвергнута -> invalid, загружена -> valid, чужое исключение -> сбой
        // The schema is rejected -> invalid, loaded -> valid, a foreign exception -> сбой (crash)
        std::string schemaActual(const xsts::SchemaAttempt& attempt)
        {
            if (attempt.unexpected)
                return "сбой";
            if (attempt.rejected)
                return "invalid";
            return "valid";
        }

        // Ошибки проверки или не well-formed -> invalid, без ошибок -> valid
        // Validation errors or not well-formed -> invalid, without errors -> valid
        std::string instanceActual(const xsts::InstanceAttempt& attempt)
        {
            if (attempt.unexpected)
                return "сбой";
            if (attempt.schemaFailed)
                return "нет схемы";
            if (attempt.rejected || attempt.errors > 0)
                return "invalid";
            return "valid";
        }
    }
}
