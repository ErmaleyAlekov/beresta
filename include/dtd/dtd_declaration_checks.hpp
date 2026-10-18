#ifndef DTD_DECLARATION_CHECKS_HPP
#define DTD_DECLARATION_CHECKS_HPP

#include "dtd/dtd_model.hpp"
#include "dtd/dtd_validator.hpp"

namespace beresta::dtd
{
    /*
    ** Ограничения валидности, которые касаются самих объявлений DTD,
    ** а не документа (раздел 3 XML 1.0). Проверяются один раз — после
    ** разбора внутреннего и внешнего подмножеств, если включена проверка:
    **  - No Duplicate Types: элемент повторён в смешанном содержимом;
    **  - One ID per Element Type и ID Attribute Default;
    **  - One Notation Per Element Type и No Notation on Empty Element;
    **  - Notation Attributes: все нотации перечисления объявлены;
    **  - No Duplicate Tokens в перечислениях;
    **  - Attribute Default Value Syntactically Correct (5-я редакция);
    **  - Notation Declared: у неразбираемой сущности объявлена нотация.
    ** Плюс замечания, накопленные при разборе (DtdModel::getValidityNotes).
    **
    ** Validity constraints that concern the DTD declarations themselves,
    ** not the document (section 3 of XML 1.0). They are checked once — after
    ** parsing the internal and external subsets, if validation is enabled:
    **  - No Duplicate Types: an element is repeated in mixed content;
    **  - One ID per Element Type and ID Attribute Default;
    **  - One Notation Per Element Type and No Notation on Empty Element;
    **  - Notation Attributes: all notations of an enumeration are declared;
    **  - No Duplicate Tokens in enumerations;
    **  - Attribute Default Value Syntactically Correct (5th edition);
    **  - Notation Declared: an unparsed entity has a declared notation.
    ** Plus the notes accumulated during parsing (DtdModel::getValidityNotes).
    */
    void checkDeclarations(const DtdModel& model, ValidityReporter& reporter);

    // Проверки одного <!ATTLIST> (dtd_attribute_checks.cpp)
    // Checks of one <!ATTLIST> (dtd_attribute_checks.cpp)
    void checkAttributeList(const DtdModel& model, const std::string& element,
        const std::vector<AttributeDeclaration>& declarations, ValidityReporter& reporter);

    // Значение по умолчанию лексически допустимо для типа атрибута
    // The default value is lexically valid for the attribute type
    bool isLegalDefault(const AttributeDeclaration& declaration);
}

#endif
