#ifndef DECLARATION_READER_HPP
#define DECLARATION_READER_HPP

#include "parser/xml_scanner.hpp"

#include <string>
#include <string_view>

namespace beresta::detail
{
    /*
    ** Общий разбор псевдо-атрибутов XML-декларации и текстовой декларации:
    ** name = "значение" | name = 'значение', перед именем обязателен пробел.
    ** Если псевдо-атрибута с таким именем нет — false, и курсор НЕ двигается:
    ** пробелы перед ним — разделитель для следующего псевдо-атрибута.
    **
    ** Common parsing of pseudo-attributes of the XML declaration and the text declaration:
    ** name = "value" | name = 'value', a space before the name is mandatory.
    ** If there is no pseudo-attribute with this name — false, and the cursor does NOT move:
    ** the spaces before it are the separator for the next pseudo-attribute.
    */
    bool readPseudoAttribute(XmlScanner& scanner, std::string_view name, std::string& value);

    /*
    ** Текстовая декларация в начале внешней сущности или внешнего DTD
    ** (продукция [77]): '<?xml' VersionInfo? EncodingDecl S? '?>'.
    ** encoding обязателен, standalone запрещён, версия — только 1.0
    ** (документ XML 1.0 не может включать сущности XML 1.1).
    ** Возвращает текст после декларации; некорректная декларация —
    ** ParseException с позицией внутри сущности.
    **
    ** The text declaration at the start of an external entity or an external DTD
    ** (production [77]): '<?xml' VersionInfo? EncodingDecl S? '?>'.
    ** encoding is mandatory, standalone is forbidden, the version is only 1.0
    ** (an XML 1.0 document cannot include XML 1.1 entities).
    ** Returns the text after the declaration; an invalid declaration is a
    ** ParseException with a position inside the entity.
    */
    std::string_view stripTextDeclaration(std::string_view text);
}

#endif
