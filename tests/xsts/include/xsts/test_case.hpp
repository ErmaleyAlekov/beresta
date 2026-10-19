#ifndef XSTS_TEST_CASE_HPP
#define XSTS_TEST_CASE_HPP

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace xsts
{
    /*
    ** Вид случая XSTS:
    **  - SCHEMA   — schemaTest: загрузить схему из одного или нескольких документов;
    **  - INSTANCE — instanceTest: проверить экземпляр по схеме своей группы.
    **
    ** The XSTS case kind:
    **  - SCHEMA   — schemaTest: load a schema from one or several documents;
    **  - INSTANCE — instanceTest: validate an instance against its group's schema.
    */
    enum class CaseKind
    {
        SCHEMA,
        INSTANCE
    };

    // Ожидаемый результат (<expected validity="..."/>)
    // The expected result (<expected validity="..."/>)
    enum class Expectation
    {
        VALID,      // схема корректна / экземпляр валиден — the schema is correct / the instance is valid
        INVALID     // схема ошибочна / экземпляр невалиден — the schema is wrong / the instance is invalid
    };

    /*
    ** Один случай набора: всё, что нужно прогонщику. Пути абсолютные —
    ** xlink:href уже разрешены от папки файла метаданных (*.testSet).
    ** features — метки проверяемых возможностей (substitutionGroup,
    ** xsi:type, block/final, redefine), найденные в тексте документов:
    ** по ним строится отдельная таблица отчёта.
    **
    ** One case of the suite: everything the runner needs. The paths are absolute —
    ** xlink:href are already resolved from the folder of the metadata file (*.testSet).
    ** features are the labels of the tested features (substitutionGroup,
    ** xsi:type, block/final, redefine) found in the text of the documents:
    ** a separate report table is built from them.
    */
    struct TestCase
    {
        std::string id;                                 // "набор/группа/имя" / "set/group/name"
        std::string name;                               // атрибут name случая / the name attribute of the case
        std::string group;                              // имя testGroup / the testGroup name
        std::string testSet;                            // имя testSet (категория отчёта) / the testSet name (a report category)
        std::string description;                        // первая documentation группы / the first documentation of the group
        CaseKind kind = CaseKind::SCHEMA;
        Expectation expected = Expectation::VALID;
        std::string status;                             // accepted / queried (из <current>) / (from <current>)
        std::string version;                            // атрибут version (пусто — любая) / the version attribute (empty — any)
        std::vector<std::filesystem::path> documents;   // документы схемы или один экземпляр
                                                        // the schema documents or one instance
        std::string location;                           // первый документ относительно корня набора
                                                        // the first document relative to the suite root
        std::string directory;                          // каталог данных: "msData/element" и т.п.
                                                        // the data directory: "msData/element" and so on
        std::vector<std::string> features;              // метки возможностей / the feature labels
    };

    /*
    ** Группа testGroup: схема (если есть schemaTest) и экземпляры,
    ** которые проверяются по ней. Хранятся индексы в Catalog::cases:
    ** вектор случаев растёт при чтении, указатели в нём были бы ненадёжны.
    **
    ** A testGroup: the schema (if there is a schemaTest) and the instances
    ** validated against it. Indexes into Catalog::cases are stored:
    ** the case vector grows while reading, pointers into it would be unreliable.
    */
    struct TestGroup
    {
        std::string name;
        std::string testSet;
        bool hasSchema = false;
        std::size_t schemaCase = 0;
        std::vector<std::size_t> instanceCases;
    };

    // Весь набор: случаи и группы в порядке файлов метаданных
    // The whole suite: the cases and groups in the order of the metadata files
    struct Catalog
    {
        std::vector<TestCase> cases;
        std::vector<TestGroup> groups;
    };

    // Вердикт по одному случаю
    // The verdict on one case
    enum class Verdict
    {
        PASSED,     // поведение совпало с ожидаемым / the behavior matched the expected one
        FAILED,     // расхождение со стандартом / a deviation from the standard
        SKIPPED,    // случай не применим (спорный, вне XSD 1.0) / the case does not apply (disputed, outside XSD 1.0)
        LIMITED     // провал из-за сознательного ограничения из README
                    // a failure caused by a deliberate limitation from the README
    };

    // Результат прогона одного случая
    // The result of running one case
    struct CaseResult
    {
        const TestCase* testCase = nullptr;
        Verdict verdict = Verdict::SKIPPED;
        std::string reason;                 // причина провала, пропуска или ограничения
                                            // the reason of the failure, the skip or the limitation
    };
}

#endif
