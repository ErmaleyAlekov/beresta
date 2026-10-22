#ifndef FAILING_CASE_EXECUTOR_HPP
#define FAILING_CASE_EXECUTOR_HPP

#include "failing/failing_case.hpp"

#include "xsts/case_runner.hpp"
#include "xsts/test_case.hpp"

#include <cstddef>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace failing
{
    /*
    ** Указатель по каталогу: случай по id и группа каждого случая.
    ** Нужен, чтобы выполнить ОДИН случай без прогона всей группы:
    ** для instanceTest надо знать схему его группы.
    **
    ** An index over the catalog: a case by id and the group of each case.
    ** It is needed to run ONE case without running the whole group:
    ** for an instanceTest the schema of its group must be known.
    */
    struct CaseIndex
    {
        std::map<std::string, std::size_t> byId;    // id -> индекс в Catalog::cases / index in Catalog::cases
        std::vector<std::size_t> groupOf;           // индекс случая -> индекс группы / case index -> group index
    };

    // Набор, открытый для выполнения случаев: каталог, указатель и корень
    // The suite opened for running cases: the catalog, the index and the root
    struct SuiteView
    {
        const xsts::Catalog& catalog;
        const CaseIndex& index;
        std::filesystem::path root;
    };

    // Строит указатель по каталогу
    // Builds the index over the catalog
    CaseIndex indexCatalog(const xsts::Catalog& catalog);

    /*
    ** Выполняет один случай теми же функциями, что и прогонщик tests/xsts
    ** (loadSchema / validateInstance и вердикты judgeSchema / judgeInstance).
    ** Это главная точка для точки останова: дальше — сама библиотека.
    **
    ** Runs one case with the same functions as the tests/xsts runner
    ** (loadSchema / validateInstance and the judgeSchema / judgeInstance verdicts).
    ** This is the main place for a breakpoint: next comes the library itself.
    */
    Execution executeCase(const SuiteView& suite, std::size_t caseIndex);

    namespace detail
    {
        // schemaTest: загрузить схему и вынести вердикт
        // schemaTest: load the schema and give the verdict
        Execution executeSchema(const xsts::TestCase& testCase, const std::filesystem::path& root);

        // instanceTest: схема группы (если есть), затем проверка экземпляра
        // instanceTest: the group schema (if any), then the instance validation
        Execution executeInstance(const SuiteView& suite, const xsts::TestGroup& group,
            const xsts::TestCase& testCase);

        // Фактическое поведение по фактам загрузки схемы
        // The actual behavior by the schema loading facts
        std::string schemaActual(const xsts::SchemaAttempt& attempt);

        // Фактическое поведение по фактам проверки экземпляра
        // The actual behavior by the instance validation facts
        std::string instanceActual(const xsts::InstanceAttempt& attempt);
    }
}

#endif
