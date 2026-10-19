#ifndef XPATH_ASSERTION_HPP
#define XPATH_ASSERTION_HPP

#include "xpath/xpath.hpp"
#include "xpath/internal/xpath_typed.hpp"

#include <string_view>

namespace beresta::xpath::detail
{
    /*
    ** ExpressionAccess — вход модуля XSD в язык утверждений XSD 1.1:
    ** компиляция в диалекте утверждений и вычисление с источником типов.
    ** Общий XPathExpression остаётся строго XPath 1.0; доступ к его
    ** закрытому конструктору и дереву разбора есть только у этого класса
    ** (дружба вместо открытого API, принцип наименьших привилегий).
    **
    ** ExpressionAccess — the entry of the XSD module into the XSD 1.1 assertion language:
    ** compilation in the assertion dialect and evaluation with a type source.
    ** The common XPathExpression stays strictly XPath 1.0; access to its
    ** private constructor and parse tree belongs only to this class
    ** (friendship instead of a public API, the principle of least privilege).
    */
    class ExpressionAccess
    {
    public:
        /*
        ** Компиляция test утверждения. namespaces — префиксы документа схемы
        ** (для имён в путях и конструкторов xs:*); пустой префикс —
        ** xpathDefaultNamespace. Синтаксическая ошибка или неподдерживаемая
        ** конструкция XPath 2.0 — DomException(XPATH_ERROR).
        **
        ** Compiles the test of an assertion. namespaces are the prefixes of the schema document
        ** (for names in paths and xs:* constructors); the empty prefix is
        ** xpathDefaultNamespace. A syntax error or an unsupported
        ** XPath 2.0 construct is DomException(XPATH_ERROR).
        */
        static XPathExpression compileAssertion(std::string_view text, const XPathContext& namespaces);

        /*
        ** Действующее логическое значение утверждения в узле context. value —
        ** значение $value (фасет xs:assertion) или nullptr (xs:assert).
        ** Ошибка вычисления — DomException(XPATH_ERROR): утверждение не выполнено.
        **
        ** The effective boolean value of the assertion at the node context. value is
        ** the value of $value (the xs:assertion facet) or nullptr (xs:assert).
        ** An evaluation error is DomException(XPATH_ERROR): the assertion is not satisfied.
        */
        static bool evaluateAssertion(const XPathExpression& test, const Node& context,
            const XPathContext& namespaces, const TypedValueSource& types, const XPathValue* value);
    };
}

#endif
