#ifndef XINCLUDE_HPP
#define XINCLUDE_HPP

#include "core/document.hpp"
#include "parser/parser_options.hpp"

#include <cstddef>
#include <string_view>

namespace beresta::xinclude
{
    // Пространство имён XInclude 1.0
    // The XInclude 1.0 namespace
    inline constexpr std::string_view XINCLUDE_NAMESPACE_URI = "http://www.w3.org/2001/XInclude";

    /*
    ** Подстановка <xi:include href="..." parse="xml|text" xpointer="..."/>.
    ** Ресурсы загружаются только через options.resolver (без него — ошибка
    ** RESOURCE_ERROR), относительные href разрешаются от options.baseUri.
    ** При ошибке загрузки используется <xi:fallback>, если он есть.
    ** XPointer: схема element(/1/2) и сокращённая форма — ID элемента.
    ** Вложенные включения обрабатываются рекурсивно, циклы обнаруживаются.
    ** Возвращает число выполненных подстановок.
    **
    ** Substitution of <xi:include href="..." parse="xml|text" xpointer="..."/>.
    ** Resources are loaded only through options.resolver (without it — the
    ** RESOURCE_ERROR error), relative hrefs are resolved against options.baseUri.
    ** On a load error <xi:fallback> is used if it exists.
    ** XPointer: the element(/1/2) scheme and the shorthand form — an element ID.
    ** Nested inclusions are processed recursively, cycles are detected.
    ** Returns the number of substitutions performed.
    */
    std::size_t processXIncludes(Document& document, const ParserOptions& options);
}

#endif
