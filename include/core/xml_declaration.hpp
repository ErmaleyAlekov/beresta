#ifndef XML_DECLARATION_HPP
#define XML_DECLARATION_HPP

#include <optional>
#include <string>

namespace beresta
{
    /*
    ** XML-декларация <?xml version="1.0" encoding="UTF-8" standalone="yes"?>.
    ** Хранится как свойство документа, а не как узел-PI: у неё строгий
    ** формат из трёх псевдо-атрибутов, и сериализатор должен уметь
    ** подменять encoding при записи в другой кодировке.
    **   version    — обязательна, вида "1.x";
    **   encoding   — пустая строка означает «не указана»;
    **   standalone — nullopt означает «не указан».
    **
    ** The XML declaration <?xml version="1.0" encoding="UTF-8" standalone="yes"?>.
    ** It is stored as a document property, not as a PI node: it has a strict
    ** format of three pseudo-attributes, and the serializer must be able to
    ** substitute encoding when writing in another encoding.
    ** version    — mandatory, of the form "1.x";
    ** encoding   — an empty string means "not specified";
    ** standalone — nullopt means "not specified".
    */
    struct XmlDeclaration
    {
        std::string version = "1.0";
        std::string encoding;
        std::optional<bool> standalone;
    };

    // Покомпонентное сравнение деклараций
    // Component-wise comparison of declarations
    bool operator==(const XmlDeclaration& left, const XmlDeclaration& right) noexcept;
    bool operator!=(const XmlDeclaration& left, const XmlDeclaration& right) noexcept;

    // Дописывает декларацию в output: псевдо-атрибуты только если они заданы
    // Appends the declaration to output: pseudo-attributes only if they are set
    void serializeXmlDeclaration(const XmlDeclaration& declaration, std::string& output);

    // Проверяет формат версии и имени кодировки; при ошибке — DomException
    // Checks the format of the version and the encoding name; on error — DomException
    void validateXmlDeclaration(const XmlDeclaration& declaration);
}

#endif
