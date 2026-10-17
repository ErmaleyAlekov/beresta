#include "xsd/internal/datatype_checks.hpp"
#include "utils/qualified_name.hpp"
#include "utils/xml_chars.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd::detail
{
    // Ограничения встроенных строковых типов: Name, NCName, NMTOKEN, ID, language, integer
    bool matchesLexicalForm(LexicalForm form, std::string_view value) noexcept
    {
        switch (form)
        {
            case LexicalForm::FREE:
                return true;
            case LexicalForm::LANGUAGE:
                return isLanguageLexical(value);
            case LexicalForm::NAME:
                return utils::isValidName(value);
            case LexicalForm::NMTOKEN:
                return utils::isValidNmtoken(value);
            case LexicalForm::INTEGER:
                return isDecimalLexical(value, true);
            default:
                return utils::isValidName(value) && value.find(':') == std::string_view::npos;
        }
    }

    // Подтег языка: 1-8 символов; первый — только буквы, остальные — буквы и цифры
    static bool isLanguagePart(std::string_view part, bool first) noexcept
    {
        if (part.empty() || part.size() > 8)
            return false;
        for (char symbol : part)
        {
            bool letter = (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z');
            if (!letter && (first || symbol < '0' || symbol > '9'))
                return false;
        }
        return true;
    }

    // language: ru, en-US, x-klingon — подтеги через дефис
    bool isLanguageLexical(std::string_view value) noexcept
    {
        std::size_t start = 0;
        bool first = true;

        while (true)
        {
            std::size_t dash = value.find('-', start);
            if (!isLanguagePart(value.substr(start, dash - start), first))
                return false;
            if (dash == std::string_view::npos)
                return true;
            start = dash + 1;
            first = false;
        }
    }

    // QName: корректное имя, а префикс (если есть) объявлен в элементе-контексте
    bool isResolvableQName(std::string_view value, const Element* context)
    {
        if (!utils::isValidQualifiedName(value))
            return false;
        std::string_view prefix = utils::splitQualifiedName(value).prefix;
        if (prefix.empty())
            return true;
        return context != nullptr && context->lookupNamespaceURI(prefix).has_value();
    }
}
