#ifndef SOURCE_LOCATION_HPP
#define SOURCE_LOCATION_HPP

#include <cstddef>

namespace beresta
{
    /*
    ** Место узла в исходном тексте (нумерация с единицы).
    ** Заполняется парсером, если включено ParserOptions::trackLocations.
    ** Нулевые значения означают «позиция неизвестна» (узел создан в коде).
    ** Позволяет сообщать об ошибках в данных уже ПОСЛЕ разбора:
    ** «неверная цена в строке 120».
    */
    struct SourceLocation
    {
        std::size_t line = 0;
        std::size_t column = 0;
    };
}

#endif
