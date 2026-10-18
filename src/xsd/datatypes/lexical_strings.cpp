#include "xsd/internal/datatype_checks.hpp"
#include "utils/qualified_name.hpp"
#include "utils/xml_chars.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::xsd::detail
{
    // Ограничения встроенных строковых типов: Name, NCName, NMTOKEN, ID, language, integer
    // Restrictions of the built-in string types: Name, NCName, NMTOKEN, ID, language, integer
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
    // A language subtag: 1-8 characters; the first one only letters, the rest letters and digits
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
    // language: ru, en-US, x-klingon — subtags separated by hyphens
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

    // QName: корректное имя, а префикс (если есть) объявлен там, где стоит значение
    // QName: a valid name, and the prefix (if any) is declared where the value is
    bool isResolvableQName(std::string_view value, const PrefixResolver& prefixes)
    {
        if (!utils::isValidQualifiedName(value))
            return false;
        std::string_view prefix = utils::splitQualifiedName(value).prefix;
        if (prefix.empty())
            return true;
        return prefixes.isBound(prefix);
    }
}
