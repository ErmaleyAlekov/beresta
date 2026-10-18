#include "utils/qualified_name.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::utils
{
    // Деление по первому двоеточию; без двоеточия всё имя считается локальным
    // Splits at the first colon; without a colon the whole name counts as local
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
    **
    ** QName = NCName (':' NCName)? — a colon cannot stand at the start,
    ** at the end or occur twice.
    */
    bool isValidQualifiedName(std::string_view name) noexcept
    {
        return isValidName(name) && hasQualifiedNameShape(name);
    }

    /*
    ** Правила двоеточий QName без повторной проверки символов: сканер уже
    ** прочитал имя по правилам Name, и второй проход по символам не нужен.
    **
    ** The QName colon rules without re-checking the characters: the scanner
    ** has already read the name by the Name rules, and a second pass is not needed.
    */
    bool hasQualifiedNameShape(std::string_view name) noexcept
    {
        std::size_t colon = name.find(':');

        if (colon == std::string_view::npos)
            return true;
        if (colon == 0 || colon + 1 == name.size())
            return false;
        return name.find(':', colon + 1) == std::string_view::npos;
    }
}
