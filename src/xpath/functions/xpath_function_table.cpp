#include "xpath/internal/xpath_eval.hpp"
#include "i18n/i18n.hpp"
#include "xpath/internal/xpath_functions.hpp"

namespace beresta::xpath::detail
{
    // Описание функции: имя, допустимое число аргументов и реализация
    // A function description: the name, the allowed number of arguments and the implementation
    struct FunctionEntry
    {
        const char* name;
        std::size_t minArguments;
        std::size_t maxArguments;
        FunctionImpl implementation;
    };

    // Значение «сколько угодно аргументов» для concat
    // The "any number of arguments" value for concat
    static constexpr std::size_t ANY_COUNT = static_cast<std::size_t>(-1);

    // Базовая библиотека функций XPath 1.0 (раздел 4 спецификации)
    // The XPath 1.0 core function library (section 4 of the specification)
    static const FunctionEntry FUNCTIONS[] = {
        {"last", 0, 0, functionLast}, {"position", 0, 0, functionPosition},
        {"count", 1, 1, functionCount}, {"id", 1, 1, functionId},
        {"local-name", 0, 1, functionLocalName}, {"namespace-uri", 0, 1, functionNamespaceUri},
        {"name", 0, 1, functionName}, {"string", 0, 1, functionString},
        {"concat", 2, ANY_COUNT, functionConcat}, {"starts-with", 2, 2, functionStartsWith},
        {"contains", 2, 2, functionContains}, {"substring-before", 2, 2, functionSubstringBefore},
        {"substring-after", 2, 2, functionSubstringAfter}, {"substring", 2, 3, functionSubstring},
        {"string-length", 0, 1, functionStringLength}, {"normalize-space", 0, 1, functionNormalizeSpace},
        {"translate", 3, 3, functionTranslate}, {"boolean", 1, 1, functionBoolean},
        {"not", 1, 1, functionNot}, {"true", 0, 0, functionTrue}, {"false", 0, 0, functionFalse},
        {"lang", 1, 1, functionLang}, {"number", 0, 1, functionNumber}, {"sum", 1, 1, functionSum},
        {"floor", 1, 1, functionFloor}, {"ceiling", 1, 1, functionCeiling}, {"round", 1, 1, functionRound}
    };

    // Поиск функции по имени и проверка числа аргументов
    // Looks up a function by name and checks the number of arguments
    XPathValue callFunction(const std::string& name, const std::vector<ExprPtr>& arguments,
        const EvalContext& context)
    {
        for (const FunctionEntry& entry : FUNCTIONS)
        {
            if (name != entry.name)
                continue;
            if (arguments.size() < entry.minArguments || arguments.size() > entry.maxArguments)
                failEvaluation(i18n::format(i18n::Message::XPATH_WRONG_ARGUMENT_COUNT, {name}));
            return entry.implementation(FunctionCall{arguments, context});
        }
        failEvaluation(i18n::format(i18n::Message::XPATH_UNKNOWN_FUNCTION, {name}));
    }
}
