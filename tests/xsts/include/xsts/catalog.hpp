#ifndef XSTS_CATALOG_HPP
#define XSTS_CATALOG_HPP

#include "xsts/test_case.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace beresta
{
    class Element;
}

namespace xsts
{
    // Пространство имён метаданных XSTS / The XSTS metadata namespace
    inline constexpr const char* XSTS_NAMESPACE = "http://www.w3.org/XML/2004/xml-schema-test-suite/";
    // Пространство имён XLink (атрибуты href) / The XLink namespace (the href attributes)
    inline constexpr const char* XLINK_NAMESPACE = "http://www.w3.org/1999/xlink";

    /*
    ** Чтение набора: suite.xml перечисляет файлы метаданных (testSetRef),
    ** каждый из них — группы testGroup со случаями schemaTest/instanceTest.
    ** Метаданные разбирает САМА «Берёста» (пространства имён, xlink).
    **
    ** Reading the suite: suite.xml lists the metadata files (testSetRef),
    ** each of them holds testGroup groups with schemaTest/instanceTest cases.
    ** The metadata is parsed by Beresta ITSELF (namespaces, xlink).
    */
    Catalog loadSuite(const std::filesystem::path& suiteRoot);

    namespace detail
    {
        // Контекст чтения одного файла метаданных
        // The context of reading one metadata file
        struct SetScope
        {
            std::filesystem::path root;         // корень набора / the suite root
            std::filesystem::path metaDir;      // папка файла метаданных / the metadata file folder
            std::string testSet;                // имя набора / the set name
            std::string description;            // описание текущей группы / the description of the current group
        };

        // Дописывает в каталог все группы одного файла *.testSet
        // Appends all groups of one *.testSet file to the catalog
        void loadTestSet(const std::filesystem::path& file, const std::filesystem::path& root,
            Catalog& catalog);

        // Читает testGroup: schemaTest и все instanceTest
        // Reads a testGroup: the schemaTest and all instanceTest
        void readGroup(const beresta::Element& group, const SetScope& scope, Catalog& catalog);

        // Строит случай из элемента schemaTest / instanceTest
        // Builds a case from a schemaTest / instanceTest element
        TestCase makeCase(const beresta::Element& test, CaseKind kind, const SetScope& scope,
            const std::string& groupName);

        // Дочерние элементы с данным локальным именем (в пространстве XSTS)
        // The child elements with the given local name (in the XSTS namespace)
        std::vector<const beresta::Element*> childElements(const beresta::Element& parent,
            std::string_view localName);

        // Первый documentation группы одной строкой (для списка расхождений)
        // The first documentation of a group as one line (for the deviation list)
        std::string groupDescription(const beresta::Element& group);

        // Значение атрибута без пространства имён или пустая строка
        // The value of an attribute without a namespace or an empty string
        std::string attributeOrEmpty(const beresta::Element& element, std::string_view name);

        // Метки возможностей по тексту документов случая
        // The feature labels by the text of the case documents
        std::vector<std::string> detectFeatures(const std::vector<std::filesystem::path>& documents);

        // Первые две части пути относительно корня ("msData/element")
        // The first two parts of the path relative to the root ("msData/element")
        std::string dataDirectory(const std::filesystem::path& file, const std::filesystem::path& root);
    }
}

#endif
