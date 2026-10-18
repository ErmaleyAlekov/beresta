#ifndef BERESTA_BENCH_WORKLOAD_HPP
#define BERESTA_BENCH_WORKLOAD_HPP

#include <cstddef>
#include <string>

namespace bench
{
    /*
    ** Набор данных для замеров. Данные генерируются детерминированно (без
    ** случайных чисел и файлов): при каждом запуске и на любой машине
    ** библиотеки получают байт в байт одинаковый текст.
    **
    ** A data set for the measurements. The data is generated deterministically
    ** (without random numbers and files): on every run and on any machine the
    ** libraries get byte-for-byte the same text.
    */
    struct Workload
    {
        std::string name;
        std::string xml;
        std::string xsd;
        std::string query;
    };

    // ---- Каталог товаров (workload_catalog.cpp) / A product catalog ----

    // Каталог: много мелких элементов с атрибутами и сущностями / A catalog: many small elements with attributes and entities
    Workload makeCatalog(std::size_t items);

    // XML Schema каталога: типы, перечисление, ID / The catalog XML Schema: types, an enumeration, ID
    std::string catalogSchema();

    // ---- Статья (workload_article.cpp) / An article ----

    // Статья: длинный русский текст со смешанным содержимым / An article: long Russian text with mixed content
    Workload makeArticle(std::size_t paragraphs);
}

#endif
