#ifndef XSD_ASSERTION_HPP
#define XSD_ASSERTION_HPP

#include "xpath/xpath.hpp"

namespace beresta::xsd
{
    /*
    ** Утверждение XSD 1.1: xs:assert у составного типа или фасет
    ** xs:assertion у простого. test компилируется при загрузке схемы на
    ** языке утверждений «Берёсты» — XPath 1.0 и подмножество XPath 2.0
    ** (if / then / else, some / every, eq ne lt le gt ge, конструкторы
    ** xs:*, exists / empty) с точной арифметикой xs:decimal; всё прочее из
    ** XPath 2.0 — ошибка загрузки схемы. context — префиксы документа схемы,
    ** под пустым префиксом — xpathDefaultNamespace (если он задан).
    ** Неизменяемо и разделяется производными типами через shared_ptr.
    **
    ** An XSD 1.1 assertion: xs:assert of a complex type or the xs:assertion
    ** facet of a simple one. test is compiled when the schema is loaded in the
    ** Beresta assertion language — XPath 1.0 and a subset of XPath 2.0
    ** (if / then / else, some / every, eq ne lt le gt ge, xs:* constructors,
    ** exists / empty) with exact xs:decimal arithmetic; everything else from
    ** XPath 2.0 is a schema loading error. context is the prefixes of the schema document,
    ** under the empty prefix — xpathDefaultNamespace (if it is given).
    ** Immutable and shared by derived types through shared_ptr.
    */
    struct Assertion
    {
        xpath::XPathExpression test;
        xpath::XPathContext context;
    };
}

#endif
