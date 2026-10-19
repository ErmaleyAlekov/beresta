#include "xsts/catalog.hpp"
#include "xsts/xsts_error.hpp"

#include "beresta.hpp"

#include <optional>
#include <string>
#include <utility>

namespace xsts
{
    namespace
    {
        /*
        ** Разбор файла метаданных «Берёстой». Файл NIST весит 9 МБ —
        ** лимит развёрнутого размера поднят, остальные настройки по умолчанию:
        ** внешние ресурсы метаданным не нужны.
        **
        ** Parses a metadata file with Beresta. The NIST file weighs 9 MB —
        ** the expanded size limit is raised, the other settings are the defaults:
        ** the metadata needs no external resources.
        */
        beresta::Document parseMetadata(const std::filesystem::path& file)
        {
            beresta::ParserOptions options;

            options.maxExpandedSize = 256u * 1024u * 1024u;
            try
            {
                return beresta::XmlParser(options).parseFile(file);
            }
            catch (const beresta::DomException& problem)
            {
                throw XstsError("не удалось прочитать " + file.u8string() + ": " + problem.what());
            }
        }

        // Значение xlink:href или ошибка окружения, если атрибута нет
        // The xlink:href value or an environment error if there is no such attribute
        std::string hrefOf(const beresta::Element& element)
        {
            std::optional<std::string> href = element.getAttributeNS(XLINK_NAMESPACE, "href");

            if (!href.has_value())
                throw XstsError("у элемента " + element.getName() + " нет xlink:href");
            return *href;
        }
    }

    /*
    ** suite.xml — корневой файл набора: testSetRef перечисляют файлы
    ** метаданных. Пути xlink:href разрешаются от папки suite.xml.
    **
    ** suite.xml is the root file of the suite: testSetRef list the metadata
    ** files. The xlink:href paths are resolved from the suite.xml folder.
    */
    Catalog loadSuite(const std::filesystem::path& suiteRoot)
    {
        std::filesystem::path index = suiteRoot / "suite.xml";

        if (!std::filesystem::is_regular_file(index))
            throw XstsError("нет файла " + index.u8string() + " (сначала make fetch)");
        beresta::Document document = parseMetadata(index);
        Catalog catalog;

        for (const beresta::Element* reference : detail::childElements(*document.getRoot(), "testSetRef"))
        {
            std::filesystem::path file = suiteRoot / std::filesystem::u8path(hrefOf(*reference));

            detail::loadTestSet(file.lexically_normal(), suiteRoot, catalog);
        }
        return catalog;
    }

    namespace detail
    {
        // Имя набора — атрибут name корневого testSet: оно и есть категория отчёта
        // The set name is the name attribute of the root testSet: it is the report category
        void loadTestSet(const std::filesystem::path& file, const std::filesystem::path& root,
            Catalog& catalog)
        {
            beresta::Document document = parseMetadata(file);
            const beresta::Element& testSet = *document.getRoot();
            SetScope scope{root, file.parent_path(), attributeOrEmpty(testSet, "name"), ""};

            for (const beresta::Element* group : childElements(testSet, "testGroup"))
                readGroup(*group, scope, catalog);
        }

        /*
        ** Группа: не больше одного schemaTest и любое число instanceTest.
        ** Экземпляр наследует метки возможностей схемы своей группы:
        ** проверка xsi:type или группы замены видна по схеме, а не по экземпляру.
        **
        ** A group: at most one schemaTest and any number of instanceTest.
        ** An instance inherits the feature labels of its group schema:
        ** an xsi:type or substitution group check is seen in the schema, not in the instance.
        */
        void readGroup(const beresta::Element& group, const SetScope& scope, Catalog& catalog)
        {
            TestGroup entry{attributeOrEmpty(group, "name"), scope.testSet, false, 0, {}};
            SetScope groupScope = scope;
            std::vector<std::string> inherited;

            groupScope.description = groupDescription(group);

            for (const beresta::Element* test : childElements(group, "schemaTest"))
            {
                entry.hasSchema = true;
                entry.schemaCase = catalog.cases.size();
                catalog.cases.push_back(makeCase(*test, CaseKind::SCHEMA, groupScope, entry.name));
                inherited = catalog.cases.back().features;
            }
            for (const beresta::Element* test : childElements(group, "instanceTest"))
            {
                TestCase instance = makeCase(*test, CaseKind::INSTANCE, groupScope, entry.name);

                for (const std::string& feature : inherited)
                    instance.features.push_back(feature);
                entry.instanceCases.push_back(catalog.cases.size());
                catalog.cases.push_back(std::move(instance));
            }
            catalog.groups.push_back(std::move(entry));
        }
    }
}
