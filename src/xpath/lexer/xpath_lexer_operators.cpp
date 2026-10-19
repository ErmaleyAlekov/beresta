#include "xpath/internal/xpath_lexer.hpp"

namespace beresta::xpath::detail
{
    // Слова-операторы XPath 1.0
    // The operator words of XPath 1.0
    static const char* const XPATH1_OPERATORS[] = {"and", "or", "mod", "div"};

    /*
    ** Слова XPath 2.0, которые утверждения XSD понимают после операнда:
    ** ветви if, части кванторов some / every и сравнения значений.
    **
    ** The XPath 2.0 words that XSD assertions understand after an operand:
    ** the if branches, the parts of the some / every quantifiers and the value comparisons.
    */
    static const char* const ASSERTION_OPERATORS[] = {
        "then", "else", "in", "satisfies", "eq", "ne", "lt", "le", "gt", "ge"
    };

    // Является ли имя после операнда оператором в текущем диалекте
    // Whether a name after an operand is an operator in the current dialect
    bool XPathLexer::isOperatorName(const std::string& name) const noexcept
    {
        for (const char* word : XPATH1_OPERATORS)
        {
            if (name == word)
                return true;
        }
        for (const char* word : ASSERTION_OPERATORS)
        {
            if (_dialect == XPathDialect::XSD_ASSERTION && name == word)
                return true;
        }
        return false;
    }
}
