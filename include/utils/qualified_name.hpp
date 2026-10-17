#ifndef QUALIFIED_NAME_HPP
#define QUALIFIED_NAME_HPP

#include <string_view>

namespace beresta
{
    // Пространство имён, навсегда связанное с префиксом "xml" (спецификация Namespaces in XML)
    inline constexpr std::string_view XML_NAMESPACE_URI = "http://www.w3.org/XML/1998/namespace";

    // Пространство имён, навсегда связанное с префиксом "xmlns"
    inline constexpr std::string_view XMLNS_NAMESPACE_URI = "http://www.w3.org/2000/xmlns/";
}

namespace beresta::utils
{
    /*
    ** Части квалифицированного имени "prefix:local".
    ** Для имени без двоеточия prefix пуст, а localName — всё имя.
    ** Обе части — представления исходной строки (без копирования).
    */
    struct QualifiedNameParts
    {
        std::string_view prefix;
        std::string_view localName;
    };

    // Делит имя по первому двоеточию
    QualifiedNameParts splitQualifiedName(std::string_view name) noexcept;

    // Корректно ли имя с точки зрения пространств имён: не более одного ':' и не по краям
    bool isValidQualifiedName(std::string_view name) noexcept;

    // Является ли атрибут объявлением пространства имён (xmlns или xmlns:*)
    bool isNamespaceDeclaration(std::string_view attributeName) noexcept;

    // Префикс, объявляемый атрибутом xmlns:p (для "xmlns" — пустая строка)
    std::string_view declaredPrefix(std::string_view attributeName) noexcept;
}

#endif
