#ifndef CONFORMANCE_CATALOG_HPP
#define CONFORMANCE_CATALOG_HPP

#include "conformance/test_case.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace beresta
{
    class Document;
    class Element;
}

namespace conformance
{
    /*
    ** Чтение каталога xmlconf.xml.
    **
    ** Каталог разбирается САМОЙ «Берёстой»: главный файл подключает
    ** наборы разных авторов через внешние общие сущности (&sun-valid; и т.п.),
    ** а пути случаев задаются через xml:base у элементов TESTCASES.
    ** Так прогонщик заодно проверяет загрузку внешних сущностей и DTD.
    **
    ** Reading the xmlconf.xml catalog.
    ** The catalog is parsed by Beresta ITSELF: the main file includes
    ** suites of different authors through external general entities (&sun-valid; and so on),
    ** and the case paths are given through xml:base on the TESTCASES elements.
    ** So the runner also tests loading external entities and DTDs.
    */
    Catalog loadCatalog(const std::filesystem::path& catalogFile);

    namespace detail
    {
        // Файл-подкаталог (внешняя сущность xmlconf.xml): его папка и текст
        // A subcatalog file (an external entity of xmlconf.xml): its folder and text
        struct EntityFile
        {
            std::filesystem::path directory;
            std::string text;
        };

        // Состояние обхода каталога: корень набора, текущая база и имя набора
        // The catalog traversal state: the suite root, the current base and the suite name
        struct WalkScope
        {
            std::filesystem::path root;
            std::filesystem::path base;
            std::string suite;
            // Файлы-подкаталоги (внешние сущности xmlconf.xml)
            // Subcatalog files (external entities of xmlconf.xml)
            const std::vector<EntityFile>* entityFiles;
        };

        // Все внешние сущности, объявленные в DOCTYPE каталога, вместе с текстом
        // All external entities declared in the catalog DOCTYPE, together with their text
        std::vector<EntityFile> entityFiles(const beresta::Document& document,
            const std::filesystem::path& root);

        // Рекурсивно обходит TESTCASES/TEST и дописывает случаи в catalog
        // Recursively traverses TESTCASES/TEST and appends the cases to catalog
        void walkCatalog(const beresta::Element& element, const WalkScope& scope,
            Catalog& catalog);

        // Строит TestCase из атрибутов элемента <TEST>
        // Builds a TestCase from the attributes of a <TEST> element
        TestCase makeTestCase(const beresta::Element& test, const WalkScope& scope);

        // Значение атрибута или пустая строка
        // The attribute value or an empty string
        std::string attributeOrEmpty(const beresta::Element& element, const char* name);
    }
}

#endif
