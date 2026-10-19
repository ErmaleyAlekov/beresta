#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd::detail
{
    // Флаг способа наследования по ключевому слову; 0 — не ключевое слово
    // The derivation method flag by its keyword; 0 — not a keyword
    static DerivationSet methodFlag(std::string_view token) noexcept
    {
        if (token == "extension")
            return DERIVATION_EXTENSION;
        if (token == "restriction")
            return DERIVATION_RESTRICTION;
        if (token == "list")
            return DERIVATION_LIST;
        if (token == "union")
            return DERIVATION_UNION;
        if (token == "substitution")
            return DERIVATION_SUBSTITUTION;
        return 0;
    }

    /*
    ** Значение block / final по схеме для схем: либо "#all" целиком,
    ** либо список слов, допустимых для компонента (маска all): у элемента
    ** в block — extension, restriction, substitution; у составного типа и
    ** в final элемента — extension, restriction; у простого типа — list,
    ** union, restriction. Регистр значим, "#all" в списке с другими
    ** словами недопустим. false — значение не по грамматике.
    **
    ** A block / final value by the schema for schemas: either "#all" alone
    ** or a list of the words allowed for the component (the all mask): for an element
    ** in block — extension, restriction, substitution; for a complex type and
    ** in an element's final — extension, restriction; for a simple type — list,
    ** union, restriction. Case matters, "#all" in a list with other
    ** words is not allowed. false — the value does not match the grammar.
    */
    static bool parseDerivationSet(std::string_view text, DerivationSet all, DerivationSet& result)
    {
        std::vector<std::string_view> tokens = utils::splitTokens(text);

        result = 0;
        if (tokens.size() == 1 && tokens.front() == "#all")
        {
            result = all;
            return true;
        }
        for (std::string_view token : tokens)
        {
            DerivationSet method = methodFlag(token);
            if ((method & all) == 0)
                return false;
            result |= method;
        }
        return true;
    }

    // Собственный атрибут компонента важнее значения по умолчанию из xs:schema
    // The component's own attribute takes precedence over the default from xs:schema
    DerivationSet SchemaBuilder::derivationAttribute(const Element& definition, std::string_view attribute,
        DerivationSet fallback, DerivationSet all) const
    {
        std::optional<std::string> value = definition.getAttribute(attribute);
        DerivationSet result = 0;

        if (!value.has_value())
            return fallback & all;
        if (!parseDerivationSet(value.value(), all, result))
            fail(definition, i18n::format(i18n::Message::SCHEMA_ATTRIBUTE_VALUE_INVALID, {attribute, value.value()}));
        return result;
    }

    // Ключевое слово XSD для способа наследования (подставляется в сообщение как есть)
    // The XSD keyword for a derivation method (substituted into the message as is)
    static std::string_view methodKeyword(DerivationSet method) noexcept
    {
        if (method == DERIVATION_EXTENSION)
            return "extension";
        if (method == DERIVATION_LIST)
            return "list";
        if (method == DERIVATION_UNION)
            return "union";
        return "restriction";
    }

    // Базовый тип с этим способом в final наследовать нельзя — это ошибка схемы
    // A base type with this method in final cannot be derived from — this is a schema error
    void SchemaBuilder::checkFinal(const Element& where, DerivationSet finalSet, const std::string& baseName,
        DerivationSet method) const
    {
        if ((finalSet & method) != 0)
            fail(where, i18n::format(i18n::Message::SCHEMA_FINAL_DERIVATION, {baseName, methodKeyword(method)}));
    }
}
