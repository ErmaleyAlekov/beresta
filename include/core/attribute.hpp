#ifndef ATTRIBUTE_HPP
#define ATTRIBUTE_HPP

#include <string>

namespace beresta
{
    /*
    ** Атрибут элемента: пара «имя — значение».
    ** Значение хранится в раскодированном виде (без &amp; и т.п.),
    ** экранирование выполняется только в момент сериализации.
    */
    struct Attribute
    {
        std::string name;
        std::string value;
    };
}

#endif
