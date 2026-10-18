#ifndef ATTRIBUTE_HPP
#define ATTRIBUTE_HPP

#include <string>

namespace beresta
{
    /*
    ** Атрибут элемента: пара «имя — значение».
    ** Значение хранится в раскодированном виде (без &amp; и т.п.),
    ** экранирование выполняется только в момент сериализации.
    **
    ** An element attribute: a "name — value" pair.
    ** The value is stored decoded (without &amp; and so on),
    ** escaping is done only at serialization time.
    */
    struct Attribute
    {
        std::string name;
        std::string value;
    };
}

#endif
