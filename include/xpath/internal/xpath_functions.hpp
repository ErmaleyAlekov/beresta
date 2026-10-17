#ifndef XPATH_FUNCTIONS_HPP
#define XPATH_FUNCTIONS_HPP

#include "xpath/internal/xpath_ast.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace beresta::xpath::detail
{
    /*
    ** Вызов функции: аргументы (ещё не вычисленные) и контекст.
    ** Каждая функция библиотеки — обычная функция с этой сигнатурой;
    ** таблица «имя -> функция» связывает их с именами (паттерн Command).
    */
    struct FunctionCall
    {
        const std::vector<ExprPtr>& arguments;
        const EvalContext& context;
    };

    using FunctionImpl = XPathValue (*)(const FunctionCall& call);

    // ---- Помощники аргументов (xpath_function_args.cpp) ----
    XPathValue argument(const FunctionCall& call, std::size_t index);
    std::string stringArgument(const FunctionCall& call, std::size_t index);
    double numberArgument(const FunctionCall& call, std::size_t index);

    // Узел-аргумент функции: первый узел множества или контекстный узел, если аргумента нет
    bool nodeArgument(const FunctionCall& call, XPathItem& item);

    // Кодовые точки строки и обратно (для substring, string-length, translate)
    std::vector<std::uint32_t> toCodePoints(const std::string& text);
    std::string fromCodePoints(const std::vector<std::uint32_t>& codes, std::size_t begin,
        std::size_t end);

    // ---- Функции над узлами (xpath_functions_nodes.cpp, xpath_functions_names.cpp) ----
    XPathValue functionLast(const FunctionCall& call);
    XPathValue functionPosition(const FunctionCall& call);
    XPathValue functionCount(const FunctionCall& call);
    XPathValue functionId(const FunctionCall& call);
    XPathValue functionLang(const FunctionCall& call);
    XPathValue functionLocalName(const FunctionCall& call);
    XPathValue functionNamespaceUri(const FunctionCall& call);
    XPathValue functionName(const FunctionCall& call);

    // ---- Строковые функции (xpath_functions_strings*.cpp) ----
    XPathValue functionString(const FunctionCall& call);
    XPathValue functionConcat(const FunctionCall& call);
    XPathValue functionStartsWith(const FunctionCall& call);
    XPathValue functionContains(const FunctionCall& call);
    XPathValue functionSubstringBefore(const FunctionCall& call);
    XPathValue functionSubstringAfter(const FunctionCall& call);
    XPathValue functionSubstring(const FunctionCall& call);
    XPathValue functionStringLength(const FunctionCall& call);
    XPathValue functionNormalizeSpace(const FunctionCall& call);
    XPathValue functionTranslate(const FunctionCall& call);

    // ---- Логические и числовые функции (xpath_functions_logic.cpp, _numbers.cpp) ----
    XPathValue functionBoolean(const FunctionCall& call);
    XPathValue functionNot(const FunctionCall& call);
    XPathValue functionTrue(const FunctionCall& call);
    XPathValue functionFalse(const FunctionCall& call);
    XPathValue functionNumber(const FunctionCall& call);
    XPathValue functionSum(const FunctionCall& call);
    XPathValue functionFloor(const FunctionCall& call);
    XPathValue functionCeiling(const FunctionCall& call);
    XPathValue functionRound(const FunctionCall& call);
}

#endif
