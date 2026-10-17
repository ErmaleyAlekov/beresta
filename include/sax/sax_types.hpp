#ifndef SAX_TYPES_HPP
#define SAX_TYPES_HPP

#include <string_view>
#include <vector>

namespace beresta
{
    /*
    ** Атрибут в SAX-событии. Все поля — представления, действительные
    ** ТОЛЬКО на время вызова обработчика; чтобы сохранить данные,
    ** скопируйте их в std::string.
    ** namespaceUri пуст для атрибутов вне пространств имён.
    */
    struct SaxAttribute
    {
        std::string_view name;
        std::string_view value;
        std::string_view namespaceUri;
        std::string_view localName;
    };

    /*
    ** Элемент в событиях startElement / endElement.
    ** В endElement список атрибутов всегда пуст.
    */
    struct SaxElement
    {
        std::string_view name;
        std::string_view namespaceUri;
        std::string_view localName;
        const std::vector<SaxAttribute>& attributes;
    };
}

#endif
