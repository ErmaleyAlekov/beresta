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
    */
    struct XmlDeclaration
    {
        std::string version = "1.0";
        std::string encoding;
        std::optional<bool> standalone;
    };

    // Покомпонентное сравнение деклараций
    bool operator==(const XmlDeclaration& left, const XmlDeclaration& right) noexcept;
    bool operator!=(const XmlDeclaration& left, const XmlDeclaration& right) noexcept;

    // Дописывает декларацию в output: псевдо-атрибуты только если они заданы
    void serializeXmlDeclaration(const XmlDeclaration& declaration, std::string& output);

    // Проверяет формат версии и имени кодировки; при ошибке — DomException
    void validateXmlDeclaration(const XmlDeclaration& declaration);
}

#endif
