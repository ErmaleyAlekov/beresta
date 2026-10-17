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
    ** Поддержаны все оси, кроме namespace, и вся базовая библиотека функций.
    ** Синтаксическая ошибка — DomException(XPATH_ERROR).
    */
    class XPathExpression
    {
    public:
        explicit XPathExpression(std::string_view expression);

        // Вычисление относительно узла context
        XPathValue evaluate(const Node& context, const XPathContext& variables = XPathContext()) const;

        // Только элементы из результата (результат обязан быть множеством узлов)
        std::vector<const Element*> selectElements(const Node& context,
            const XPathContext& variables = XPathContext()) const;
        std::vector<Element*> selectElements(Node& context,
            const XPathContext& variables = XPathContext()) const;

        // Исходный текст выражения
        const std::string& getSource() const noexcept;

    private:
        std::string _source;
        std::shared_ptr<const detail::Expr> _root;
    };

    // Быстрые обёртки: компиляция и вычисление одной строкой
    std::vector<Element*> selectElements(Node& context, std::string_view expression);
    std::vector<const Element*> selectElements(const Node& context, std::string_view expression);
    std::string evaluateString(const Node& context, std::string_view expression);
}

#endif
