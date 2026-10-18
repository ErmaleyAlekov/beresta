#include "dtd/dtd_validator.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_chars.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::dtd
{
    /*
    ** Сначала — каждый указанный атрибут должен быть объявлен; затем —
    ** каждое объявление проверяется на обязательность, фиксированность и тип.
    **
    ** First, every specified attribute must be declared; then
    ** every declaration is checked for being required, fixed and for the type.
    */
    void DtdValidator::checkAttributes(std::string_view element,
        const std::vector<Attribute>& attributes)
    {
        const std::vector<AttributeDeclaration>* declarations = _model.findAttributes(element);

        for (const Attribute& attribute : attributes)
        {
            if (_model.findAttribute(element, attribute.name) == nullptr)
                _reporter.reportValidity(i18n::format(i18n::Message::DTD_ATTRIBUTE_UNDECLARED, {attribute.name, element}));
        }
        if (declarations == nullptr)
            return;
        for (const AttributeDeclaration& declaration : *declarations)
        {
            const Attribute* present = nullptr;
            for (const Attribute& attribute : attributes)
            {
                if (attribute.name == declaration.name)
                    present = &attribute;
            }
            checkDeclaredAttribute(declaration, present);
        }
    }

    // Обязательность, фиксированное значение и проверка по типу
    // Required, the fixed value and the check by type
    void DtdValidator::checkDeclaredAttribute(const AttributeDeclaration& declaration,
        const Attribute* present)
    {
        if (present == nullptr)
        {
            if (declaration.defaultKind == DefaultKind::REQUIRED)
                _reporter.reportValidity(i18n::format(i18n::Message::ATTRIBUTE_REQUIRED_MISSING, {declaration.name}));
            return;
        }
        if (declaration.defaultKind == DefaultKind::FIXED && present->value != declaration.defaultValue)
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_ATTRIBUTE_FIXED, {declaration.name, declaration.defaultValue}));
        checkValueByType(declaration, present->value);
    }

    // Перечисления сверяются со списком; списковые типы делятся на токены
    // Enumerations are matched against the list; list types are split into tokens
    void DtdValidator::checkValueByType(const AttributeDeclaration& declaration,
        std::string_view value)
    {
        AttributeType type = declaration.type;

        if (type == AttributeType::CDATA)
            return;
        if (type == AttributeType::ENUMERATION || type == AttributeType::NOTATION)
        {
            for (const std::string& allowed : declaration.allowedValues)
            {
                if (allowed == value)
                    return;
            }
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_NOT_IN_ENUMERATION, {value, declaration.name}));
            return;
        }
        // После нормализации токены разделены ТОЛЬКО пробелами: таб из &#9; — часть токена
        // After normalization the tokens are separated ONLY by spaces: a tab from &#9; is part of a token
        std::vector<std::string_view> tokens = utils::splitOnSpaces(value);
        bool isList = type == AttributeType::IDREFS || type == AttributeType::NMTOKENS
            || type == AttributeType::ENTITIES;
        if (tokens.empty() || (!isList && tokens.size() > 1))
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_ATTRIBUTE_VALUE_INVALID, {declaration.name}));
        for (std::string_view token : tokens)
            checkToken(declaration, token);
    }

    // Один токен: синтаксис по типу, уникальность ID, ссылки IDREF, сущности ENTITY
    // One token: the syntax by type, ID uniqueness, IDREF references, ENTITY entities
    void DtdValidator::checkToken(const AttributeDeclaration& declaration, std::string_view token)
    {
        AttributeType type = declaration.type;
        bool nmtoken = type == AttributeType::NMTOKEN || type == AttributeType::NMTOKENS;

        if (nmtoken || !utils::isValidName(token))
        {
            if (!utils::isValidNmtoken(token) || !nmtoken)
                _reporter.reportValidity(i18n::format(i18n::Message::DTD_TOKEN_INVALID, {token, declaration.name}));
            return;
        }
        if (_namespaceAware && token.find(':') != std::string_view::npos)
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_TOKEN_NOT_NCNAME, {token}));
        if (type == AttributeType::ID && !_ids.insert(std::string(token)).second)
            _reporter.reportValidity(i18n::format(i18n::Message::ID_DUPLICATE, {token}));
        if (type == AttributeType::IDREF || type == AttributeType::IDREFS)
            _references.push_back(std::string(token));
        if (type != AttributeType::ENTITY && type != AttributeType::ENTITIES)
            return;
        const EntityDeclaration* entity = _model.findEntity(token);
        if (entity == nullptr || entity->notation.empty())
            _reporter.reportValidity(i18n::format(i18n::Message::DTD_NOT_UNPARSED_ENTITY, {token}));
    }
}
