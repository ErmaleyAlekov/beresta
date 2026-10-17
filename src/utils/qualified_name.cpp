#include "utils/qualified_name.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::utils
{
    // Деление по первому двоеточию; без двоеточия всё имя считается локальным
    QualifiedNameParts splitQualifiedName(std::string_view name) noexcept
    {
        std::size_t colon = name.find(':');

        if (colon == std::string_view::npos)
            return QualifiedNameParts{std::string_view(), name};
        return QualifiedNameParts{name.substr(0, colon), name.substr(colon + 1)};
    }

    /*
    ** QName = NCName (':' NCName)? — двоеточие не может стоять в начале,
    ** в конце или встречаться дважды.
    */
    bool isValidQualifiedName(std::string_view name) noexcept
    {
        std::size_t colon = name.find(':');

        if (!isValidName(name))
            return false;
        if (colon == std::string_view::npos)
            return true;
        if (colon == 0 || colon + 1 == name.size())
            return false;
        return name.find(':', colon + 1) == std::string_view::npos;
    }

    // xmlns="..." объявляет пространство по умолчанию, xmlns:p="..." — префикс p
    bool isNamespaceDeclaration(std::string_view attributeName) noexcept
    {
        if (attributeName == "xmlns")
            return true;
        return attributeName.substr(0, 6) == "xmlns:";
    }

    // Для "xmlns:soap" возвращает "soap", для "xmlns" — пустое представление
    std::string_view declaredPrefix(std::string_view attributeName) noexcept
    {
        if (attributeName.size() <= 6)
            return std::string_view();
        return attributeName.substr(6);
    }
}
