#include "utils/qualified_name.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::utils
{
    // xmlns="..." объявляет пространство по умолчанию, xmlns:p="..." — префикс p
    // xmlns="..." declares the default namespace, xmlns:p="..." declares the prefix p
    bool isNamespaceDeclaration(std::string_view attributeName) noexcept
    {
        if (attributeName == "xmlns")
            return true;
        return attributeName.substr(0, 6) == "xmlns:";
    }

    // Для "xmlns:soap" возвращает "soap", для "xmlns" — пустое представление
    // Returns "soap" for "xmlns:soap", an empty view for "xmlns"
    std::string_view declaredPrefix(std::string_view attributeName) noexcept
    {
        if (attributeName.size() <= 6)
            return std::string_view();
        return attributeName.substr(6);
    }

    /*
    ** Объявляет ли атрибут с именем name префикс prefix: "xmlns" — пустой
    ** префикс (пространство по умолчанию), "xmlns:p" — префикс p. Сравнение
    ** без сборки строки "xmlns:" + prefix: поиск пространства имён идёт для
    ** каждого элемента, и лишнее выделение памяти здесь заметно.
    **
    ** Whether the attribute named name declares the prefix prefix: "xmlns" is the empty
    ** prefix (the default namespace), "xmlns:p" is the prefix p. A comparison
    ** without building the string "xmlns:" + prefix: the namespace lookup runs for
    ** every element, and an extra memory allocation here is noticeable.
    */
    bool declaresPrefix(std::string_view name, std::string_view prefix) noexcept
    {
        if (prefix.empty())
            return name == "xmlns";
        return name.size() == prefix.size() + 6 && name.substr(0, 6) == "xmlns:" && name.substr(6) == prefix;
    }
}
