#ifndef XSTS_CASE_RUNNER_HPP
#define XSTS_CASE_RUNNER_HPP

#include "xsts/limitations.hpp"
#include "xsts/test_case.hpp"

#include "core/document.hpp"
#include "xsd/schema.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace xsts
{
    /*
    ** Итог загрузки схемы — «сырые факты» без оценки.
    ** rejected — библиотека отвергла схему (DomException),
    ** unexpected — брошено НЕ исключение библиотеки (сбой самой библиотеки).
    **
    ** The outcome of loading a schema — "raw facts" without an assessment.
    ** rejected — the library rejected the schema (DomException),
    ** unexpected — a NON-library exception was thrown (a failure of the library itself).
    */
    struct SchemaAttempt
    {
        std::shared_ptr<const beresta::xsd::Schema> schema;
        bool rejected = false;
        bool unexpected = false;
        std::string message;
    };

    // Итог проверки экземпляра: не разобран, сбой или число ошибок валидности
    // The outcome of validating an instance: not parsed, a failure or the number of validity errors
    struct InstanceAttempt
    {
        bool rejected = false;              // экземпляр не разобран (не well-formed)
                                            // the instance is not parsed (not well-formed)
        bool schemaFailed = false;          // схема по подсказкам xsi:schemaLocation не загрузилась
                                            // the schema by the xsi:schemaLocation hints did not load
        bool unexpected = false;
        std::size_t errors = 0;
        std::string message;                // первая ошибка — для отчёта / the first error — for the report
    };

    /*
    ** GroupRunner — прогон одной группы (паттерн Template Method в
    ** функциональном виде: «загрузить схему -> вердикт schemaTest ->
    ** проверить каждый экземпляр -> вердикт instanceTest -> ограничения»).
    ** Схема группы загружается один раз и разделяется всеми экземплярами.
    ** Корень набора нужен резолверу: include/import читаются только изнутри
    ** набора, сетевые адреса резолвер не пропускает.
    **
    ** GroupRunner — running one group (the Template Method pattern in a
    ** functional form: "load the schema -> the schemaTest verdict ->
    ** validate each instance -> the instanceTest verdict -> limitations").
    ** The group schema is loaded once and shared by all instances.
    ** The suite root is needed by the resolver: include/import are read only from inside
    ** the suite, the resolver does not let network addresses through.
    */
    class GroupRunner
    {
    public:
        GroupRunner(std::filesystem::path suiteRoot, const Limitations& limitations);

        // Прогоняет группу и дописывает результаты всех её случаев
        // Runs a group and appends the results of all its cases
        void run(const Catalog& catalog, const TestGroup& group, std::vector<CaseResult>& results) const;

    private:
        // Проверяет один экземпляр по схеме группы (или по подсказкам xsi:schemaLocation)
        // Validates one instance against the group schema (or by the xsi:schemaLocation hints)
        CaseResult runInstance(const TestCase& testCase, const SchemaAttempt& schema,
            const CaseResult* schemaResult) const;

        // Пропуск, ограничение или итоговый вердикт
        // A skip, a limitation or the final verdict
        CaseResult finish(const TestCase& testCase, CaseResult judged) const;

        std::filesystem::path _suiteRoot;
        const Limitations& _limitations;
    };

    // Загружает схему из документов случая через Schema::fromDocuments
    // Loads a schema from the case documents through Schema::fromDocuments
    SchemaAttempt loadSchema(const std::vector<std::filesystem::path>& documents,
        const std::filesystem::path& suiteRoot);

    /*
    ** Разбирает экземпляр и проверяет его SchemaValidator. Пустой schema —
    ** группа без schemaTest, схема берётся из подсказок; иначе к схеме
    ** группы (groupDocuments) добавляются подсказки для новых пространств имён.
    **
    ** Parses an instance and validates it with SchemaValidator. An empty schema —
    ** a group without a schemaTest, the schema is taken from the hints; otherwise hints
    ** for new namespaces are added to the group schema (groupDocuments).
    */
    InstanceAttempt validateInstance(const std::filesystem::path& file,
        std::shared_ptr<const beresta::xsd::Schema> schema,
        const std::vector<std::filesystem::path>& groupDocuments, const std::filesystem::path& suiteRoot);

    // Схема по подсказкам xsi:schemaLocation / xsi:noNamespaceSchemaLocation экземпляра
    // A schema by the xsi:schemaLocation / xsi:noNamespaceSchemaLocation hints of the instance
    SchemaAttempt loadHintedSchema(const beresta::Document& instance,
        const std::filesystem::path& file, const std::filesystem::path& suiteRoot);

    // Схема группы, дополненная подсказками экземпляра; пустая схема — дополнять нечем
    // The group schema extended by the instance hints; an empty schema — nothing to extend with
    SchemaAttempt extendWithHints(const std::vector<std::filesystem::path>& groupDocuments,
        const beresta::Document& instance, const std::filesystem::path& file,
        const std::filesystem::path& suiteRoot);

    // Вердикт schemaTest по фактам загрузки
    // The schemaTest verdict by the loading facts
    CaseResult judgeSchema(const TestCase& testCase, const SchemaAttempt& attempt);

    // Вердикт instanceTest по фактам проверки
    // The instanceTest verdict by the validation facts
    CaseResult judgeInstance(const TestCase& testCase, const InstanceAttempt& attempt);
}

#endif
