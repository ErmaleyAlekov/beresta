#include "core/xml_declaration.hpp"
#include "exception/dom_exception.hpp"

namespace beresta
{
    // Декларации равны, если совпадают все три псевдо-атрибута
    bool operator==(const XmlDeclaration& left, const XmlDeclaration& right) noexcept
    {
        if (left.version != right.version || left.encoding != right.encoding)
            return false;
        return left.standalone == right.standalone;
    }

    // Неравенство — отрицание равенства
    bool operator!=(const XmlDeclaration& left, const XmlDeclaration& right) noexcept
    {
        return !(left == right);
    }

    // Формат строго по стандарту; пробел — только перед каждым псевдо-атрибутом
    void serializeXmlDeclaration(const XmlDeclaration& declaration, std::string& output)
    {
        output += "<?xml version=\"";
        output += declaration.version;
        output += '"';
        if (!declaration.encoding.empty())
        {
            output += " encoding=\"";
            output += declaration.encoding;
            output += '"';
        }
        if (declaration.standalone.has_value())
        {
            if (declaration.standalone.value())
                output += " standalone=\"yes\"";
            else
                output += " standalone=\"no\"";
        }
        output += "?>";
    }

    // Символ имени кодировки: латиница, цифры, '.', '_', '-' (EncName из XML 1.0)
    static bool isEncodingChar(char symbol, bool first) noexcept
    {
        bool letter = (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z');

        if (first)
            return letter;
        if (letter || (symbol >= '0' && symbol <= '9'))
            return true;
        return symbol == '.' || symbol == '_' || symbol == '-';
    }

    /*
    ** Версия: "1." и хотя бы одна цифра (VersionNum).
    ** Кодировка: пусто либо EncName — латинская буква и далее допустимые символы.
    */
    void validateXmlDeclaration(const XmlDeclaration& declaration)
    {
        const std::string& version = declaration.version;
        bool versionOk = version.size() > 2 && version.compare(0, 2, "1.") == 0;

        for (std::size_t index = 2; versionOk && index < version.size(); ++index)
            versionOk = version[index] >= '0' && version[index] <= '9';
        if (!versionOk)
            throw DomException(DomErrorCode::INVALID_CONTENT,
                "недопустимая версия XML: \"" + version + "\"");
        for (std::size_t index = 0; index < declaration.encoding.size(); ++index)
        {
            if (!isEncodingChar(declaration.encoding[index], index == 0))
                throw DomException(DomErrorCode::INVALID_CONTENT,
                    "недопустимое имя кодировки: \"" + declaration.encoding + "\"");
        }
    }
}
