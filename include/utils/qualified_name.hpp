#ifndef QUALIFIED_NAME_HPP
#define QUALIFIED_NAME_HPP

#include <string_view>

namespace beresta
{
    // Пространство имён, навсегда связанное с префиксом "xml" (спецификация Namespaces in XML)
    // The namespace permanently bound to the "xml" prefix (the Namespaces in XML specification)
    inline constexpr std::string_view XML_NAMESPACE_URI = "http://www.w3.org/XML/1998/namespace";

    // Пространство имён, навсегда связанное с префиксом "xmlns"
    // The namespace permanently bound to the "xmlns" prefix
    inline constexpr std::string_view XMLNS_NAMESPACE_URI = "http://www.w3.org/2000/xmlns/";
}

namespace beresta::utils
{
    /*
    ** Части квалифицированного имени "prefix:local".
    ** Для имени без двоеточия prefix пуст, а localName — всё имя.
    ** Обе части — представления исходной строки (без копирования).
    **
    ** Parts of the qualified name "prefix:local".
    ** For a name without a colon prefix is empty, and localName is the whole name.
    ** Both parts are views of the source string (without copying).
    */
    struct QualifiedNameParts
    {
        std::string_view prefix;
        std::string_view localName;
    };

    // Делит имя по первому двоеточию
    // Splits the name at the first colon
    QualifiedNameParts splitQualifiedName(std::string_view name) noexcept;

    // Корректно ли имя с точки зрения пространств имён: не более одного ':' и не по краям
    // Whether the name is valid in terms of namespaces: at most one ':' and not at the edges
    bool isValidQualifiedName(std::string_view name) noexcept;

    // Форма QName для имени, которое УЖЕ проверено как Name: только правила двоеточий
    // The QName shape for a name ALREADY checked as a Name: only the colon rules
    bool hasQualifiedNameShape(std::string_view name) noexcept;

    // Является ли атрибут объявлением пространства имён (xmlns или xmlns:*)
    // Whether the attribute is a namespace declaration (xmlns or xmlns:*)
    bool isNamespaceDeclaration(std::string_view attributeName) noexcept;

    // Префикс, объявляемый атрибутом xmlns:p (для "xmlns" — пустая строка)
    // The prefix declared by the xmlns:p attribute (for "xmlns" — an empty string)
    std::string_view declaredPrefix(std::string_view attributeName) noexcept;

    // Объявляет ли атрибут name префикс prefix (пустой префикс — атрибут "xmlns")
    // Whether the attribute name declares the prefix prefix (the empty prefix is the "xmlns" attribute)
    bool declaresPrefix(std::string_view name, std::string_view prefix) noexcept;
}

#endif
