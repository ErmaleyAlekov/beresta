#include "dtd/dtd_declaration_checks.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_chars.hpp"
#include "utils/xml_tokens.hpp"

#include <set>

namespace beresta::dtd
{
    namespace
    {
        // Непустой список токенов через пробел; names — каждый токен Name, иначе Nmtoken
        // A non-empty space-separated list of tokens; names — every token is a Name, otherwise an Nmtoken
        bool isTokenList(std::string_view value, bool names)
        {
            std::vector<std::string_view> tokens = utils::splitOnSpaces(value);

            if (tokens.empty())
                return false;
            for (std::string_view token : tokens)
            {
                if (names && !utils::isValidName(token))
                    return false;
                if (!names && !utils::isValidNmtoken(token))
                    return false;
            }
            return true;
        }

        // VC: No Duplicate Tokens и VC: Notation Attributes (все нотации объявлены)
        // VC: No Duplicate Tokens and VC: Notation Attributes (all notations are declared)
        void checkEnumeration(const DtdModel& model, const AttributeDeclaration& declaration,
            ValidityReporter& reporter)
        {
            std::set<std::string, std::less<>> seen;

            for (const std::string& value : declaration.allowedValues)
            {
                if (!seen.insert(value).second)
                    reporter.reportValidity(i18n::format(i18n::Message::DTD_ENUMERATION_DUPLICATE, {value, declaration.name}));
                if (declaration.type == AttributeType::NOTATION && !model.hasNotation(value))
                    reporter.reportValidity(i18n::format(i18n::Message::DTD_NOTATION_UNDECLARED_ATTRIBUTE, {value, declaration.name}));
            }
        }

        // Одно объявление атрибута: ID-умолчание, перечисление, допустимость умолчания
        // One attribute declaration: the ID default, the enumeration, the validity of the default
        void checkAttribute(const DtdModel& model, const AttributeDeclaration& declaration,
            ValidityReporter& reporter)
        {
            bool hasDefault = declaration.defaultKind == DefaultKind::VALUE
                || declaration.defaultKind == DefaultKind::FIXED;

            if (declaration.type == AttributeType::ID && hasDefault)
                reporter.reportValidity(i18n::format(i18n::Message::DTD_ID_DEFAULT, {declaration.name}));
            checkEnumeration(model, declaration, reporter);
            if (hasDefault && !isLegalDefault(declaration))
                reporter.reportValidity(i18n::format(i18n::Message::DTD_DEFAULT_INVALID, {declaration.defaultValue, declaration.name}));
        }
    }

    /*
    ** Ограничения на весь <!ATTLIST> элемента: не больше одного атрибута
    ** типа ID и одного типа NOTATION, а NOTATION-атрибут не может быть
    ** у элемента, объявленного EMPTY.
    **
    ** Constraints on the whole <!ATTLIST> of an element: at most one attribute
    ** of type ID and one of type NOTATION, and a NOTATION attribute cannot belong
    ** to an element declared EMPTY.
    */
    void checkAttributeList(const DtdModel& model, const std::string& element,
        const std::vector<AttributeDeclaration>& declarations, ValidityReporter& reporter)
    {
        std::size_t ids = 0;
        std::size_t notations = 0;
        const ElementDeclaration* declared = model.findElement(element);

        for (const AttributeDeclaration& declaration : declarations)
        {
            if (declaration.type == AttributeType::ID)
                ++ids;
            if (declaration.type == AttributeType::NOTATION)
                ++notations;
            checkAttribute(model, declaration, reporter);
        }
        if (ids > 1)
            reporter.reportValidity(i18n::format(i18n::Message::DTD_SEVERAL_IDS, {element}));
        if (notations > 1)
            reporter.reportValidity(i18n::format(i18n::Message::DTD_SEVERAL_NOTATIONS, {element}));
        if (notations > 0 && declared != nullptr && declared->kind == ContentKind::EMPTY_CONTENT)
            reporter.reportValidity(i18n::format(i18n::Message::DTD_NOTATION_ON_EMPTY, {element}));
    }

    // Лексические ограничения типа (раздел 3.3.2, 5-я редакция) для значения по умолчанию
    // The lexical constraints of the type (section 3.3.2, 5th edition) for the default value
    bool isLegalDefault(const AttributeDeclaration& declaration)
    {
        const std::string& value = declaration.defaultValue;
        AttributeType type = declaration.type;

        if (type == AttributeType::CDATA)
            return true;
        if (type == AttributeType::ID || type == AttributeType::IDREF || type == AttributeType::ENTITY)
            return utils::isValidName(value);
        if (type == AttributeType::IDREFS || type == AttributeType::ENTITIES)
            return isTokenList(value, true);
        if (type == AttributeType::NMTOKEN)
            return utils::isValidNmtoken(value);
        if (type == AttributeType::NMTOKENS)
            return isTokenList(value, false);
        for (const std::string& allowed : declaration.allowedValues)
        {
            if (allowed == value)
                return true;
        }
        return false;
    }
}
