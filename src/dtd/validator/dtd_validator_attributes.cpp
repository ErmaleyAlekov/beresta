#include "dtd/dtd_validator.hpp"
#include "utils/xml_chars.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::dtd
{
    /*
    ** Сначала — каждый указанный атрибут должен быть объявлен; затем —
    ** каждое объявление проверяется на обязательность, фиксированность и тип.
    */
    void DtdValidator::checkAttributes(std::string_view element,
        const std::vector<Attribute>& attributes)
    {
        const std::vector<AttributeDeclaration>* declarations = _model.findAttributes(element);

        for (const Attribute& attribute : attributes)
        {
            if (_model.findAttribute(element, attribute.name) == nullptr)
                _reporter.reportValidity("атрибут \"" + attribute.name + "\" элемента <"
                    + std::string(element) + "> не объявлен в DTD");
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
    void DtdValidator::checkDeclaredAttribute(const AttributeDeclaration& declaration,
        const Attribute* present)
    {
        if (present == nullptr)
        {
            if (declaration.defaultKind == DefaultKind::REQUIRED)
                _reporter.reportValidity("отсутствует обязательный атрибут \"" + declaration.name + "\"");
            return;
        }
        if (declaration.defaultKind == DefaultKind::FIXED && present->value != declaration.defaultValue)
            _reporter.reportValidity("атрибут \"" + declaration.name + "\" должен иметь значение \""
                + declaration.defaultValue + "\"");
        checkValueByType(declaration, present->value);
    }

    // Перечисления сверяются со списком; списковые типы делятся на токены
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
            _reporter.reportValidity("значение \"" + std::string(value) + "\" атрибута \""
                + declaration.name + "\" не входит в перечисление");
            return;
        }
        std::vector<std::string_view> tokens = utils::splitTokens(value);
        bool isList = type == AttributeType::IDREFS || type == AttributeType::NMTOKENS
            || type == AttributeType::ENTITIES;
        if (tokens.empty() || (!isList && tokens.size() > 1))
            _reporter.reportValidity("недопустимое значение атрибута \"" + declaration.name + "\"");
        for (std::string_view token : tokens)
            checkToken(declaration, token);
    }

    // Один токен: синтаксис по типу, уникальность ID, ссылки IDREF, сущности ENTITY
    void DtdValidator::checkToken(const AttributeDeclaration& declaration, std::string_view token)
    {
        AttributeType type = declaration.type;
        bool nmtoken = type == AttributeType::NMTOKEN || type == AttributeType::NMTOKENS;

        if (nmtoken || !utils::isValidName(token))
        {
            if (!utils::isValidNmtoken(token) || !nmtoken)
                _reporter.reportValidity("\"" + std::string(token) + "\" — недопустимое значение для "
                    "атрибута \"" + declaration.name + "\"");
            return;
        }
        if (type == AttributeType::ID && !_ids.insert(std::string(token)).second)
            _reporter.reportValidity("повторяющийся ID \"" + std::string(token) + "\"");
        if (type == AttributeType::IDREF || type == AttributeType::IDREFS)
            _references.push_back(std::string(token));
        if (type != AttributeType::ENTITY && type != AttributeType::ENTITIES)
            return;
        const EntityDeclaration* entity = _model.findEntity(token);
        if (entity == nullptr || entity->notation.empty())
            _reporter.reportValidity("\"" + std::string(token) + "\" не является неразбираемой сущностью");
    }
}
