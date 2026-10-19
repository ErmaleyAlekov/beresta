#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Значение block / final: "#all" или список слов extension,
    ** restriction, list, union, substitution. Слова, неприменимые к
    ** компоненту, отбрасываются маской all (так же, как #all раскрывается
    ** только в применимые способы).
    **
    ** A block / final value: "#all" or a list of the words extension,
    ** restriction, list, union, substitution. Words not applicable to the
    ** component are dropped by the all mask (just as #all expands
    ** only into the applicable methods).
    */
    static DerivationSet parseDerivationSet(std::string_view text, DerivationSet all)
    {
        DerivationSet result = 0;

        for (std::string_view token : utils::splitTokens(text))
        {
            if (token == "#all")
                result = all;
            else if (token == "extension")
                result |= DERIVATION_EXTENSION;
            else if (token == "restriction")
                result |= DERIVATION_RESTRICTION;
            else if (token == "list")
                result |= DERIVATION_LIST;
            else if (token == "union")
                result |= DERIVATION_UNION;
            else if (token == "substitution")
                result |= DERIVATION_SUBSTITUTION;
        }
        return result & all;
    }

    // Собственный атрибут компонента важнее значения по умолчанию из xs:schema
    // The component's own attribute takes precedence over the default from xs:schema
    DerivationSet derivationAttribute(const Element& definition, std::string_view attribute,
        DerivationSet fallback, DerivationSet all)
    {
        std::optional<std::string> value = definition.getAttribute(attribute);

        if (!value.has_value())
            return fallback & all;
        return parseDerivationSet(value.value(), all);
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
