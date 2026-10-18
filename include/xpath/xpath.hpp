#ifndef XPATH_HPP
#define XPATH_HPP

#include "core/element.hpp"
#include "core/node.hpp"
#include "xpath/xpath_value.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::xpath
{
    namespace detail
    {
        class Expr;
    }

    /*
    ** Контекст вычисления: префиксы пространств имён для имён в выражении
    ** (в XPath 1.0 имя без префикса означает «вне пространств имён»,
    ** поэтому документ с xmlns="..." требует префикса в выражении)
    ** и значения переменных $имя.
    **
    ** The evaluation context: namespace prefixes for the names in the expression
    ** (in XPath 1.0 a name without a prefix means "in no namespace",
    ** so a document with xmlns="..." requires a prefix in the expression)
    ** and the values of the $name variables.
    */
    struct XPathContext
    {
        std::map<std::string, std::string, std::less<>> namespaces;
        std::map<std::string, XPathValue, std::less<>> variables;
    };

    /*
    ** XPathExpression — скомпилированное выражение XPath 1.0.
    ** Компиляция выполняется один раз в конструкторе, вычислять можно
    ** многократно и из разных потоков (выражение неизменяемо).
    ** Поддержаны все 13 осей (включая namespace) и вся базовая библиотека функций.
    ** Синтаксическая ошибка — DomException(XPATH_ERROR).
    **
    ** XPathExpression — a compiled XPath 1.0 expression.
    ** It is compiled once in the constructor and can be evaluated
    ** many times and from different threads (the expression is immutable).
    ** All 13 axes (including namespace) and the whole core function library are supported.
    ** A syntax error is DomException(XPATH_ERROR).
    */
    class XPathExpression
    {
    public:
        explicit XPathExpression(std::string_view expression);

        // Вычисление относительно узла context
        // Evaluation relative to the node context
        XPathValue evaluate(const Node& context, const XPathContext& variables = XPathContext()) const;

        // Только элементы из результата (результат обязан быть множеством узлов)
        // Only the elements of the result (the result must be a node set)
        std::vector<const Element*> selectElements(const Node& context,
            const XPathContext& variables = XPathContext()) const;
        std::vector<Element*> selectElements(Node& context,
            const XPathContext& variables = XPathContext()) const;

        // Исходный текст выражения
        // The source text of the expression
        const std::string& getSource() const noexcept;

    private:
        std::string _source;
        std::shared_ptr<const detail::Expr> _root;
    };

    // Быстрые обёртки: компиляция и вычисление одной строкой
    // Quick wrappers: compilation and evaluation in one line
    std::vector<Element*> selectElements(Node& context, std::string_view expression);
    std::vector<const Element*> selectElements(const Node& context, std::string_view expression);
    std::string evaluateString(const Node& context, std::string_view expression);
}

#endif
