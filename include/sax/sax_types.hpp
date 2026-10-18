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
    **
    ** An attribute in a SAX event. All fields are views valid
    ** ONLY during the handler call; to keep the data,
    ** copy it into std::string.
    ** namespaceUri is empty for attributes in no namespace.
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
    **
    ** An element in the startElement / endElement events.
    ** In endElement the attribute list is always empty.
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
