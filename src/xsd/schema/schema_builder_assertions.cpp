#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "xpath/internal/xpath_assertion.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** Пространство имён для имён элементов без префикса в утверждении
    ** (xpathDefaultNamespace XSD 1.1): атрибут самого утверждения, иначе
    ** xs:schema; ##targetNamespace — целевое пространство схемы,
    ** ##defaultNamespace — пространство xmlns="…" в этой точке схемы,
    ** ##local и отсутствие атрибута — без пространства (как в XPath 1.0).
    **
    ** The namespace for element names without a prefix in an assertion
    ** (XSD 1.1 xpathDefaultNamespace): the attribute of the assertion itself, otherwise
    ** of xs:schema; ##targetNamespace — the target namespace of the schema,
    ** ##defaultNamespace — the xmlns="…" namespace at this point of the schema,
    ** ##local and no attribute — no namespace (as in XPath 1.0).
    */
    static std::string defaultElementNamespace(const Element& definition, const SchemaSource& source)
    {
        std::string value = attributeOr(definition, "xpathDefaultNamespace",
            attributeOr(*source.root, "xpathDefaultNamespace", "##local"));

        if (value == "##local")
            return std::string();
        if (value == "##targetNamespace")
            return source.targetNamespace;
        if (value == "##defaultNamespace")
            return definition.lookupNamespaceURI("").value_or("");
        return value;
    }

    /*
    ** Одно утверждение (xs:assert или фасет xs:assertion): test компилируется
    ** сразу в диалекте утверждений. Синтаксическая ошибка, неизвестная
    ** функция или любая конструкция XPath 2.0 вне поддерживаемого набора —
    ** ошибка загрузки схемы со строкой в документе схемы, а не «тихая»
    ** истина при проверке.
    **
    ** One assertion (xs:assert or the xs:assertion facet): test is compiled
    ** at once in the assertion dialect. A syntax error, an unknown
    ** function or any XPath 2.0 construct outside the supported set is
    ** a schema loading error with the line in the schema document, not a "silent"
    ** truth during validation.
    */
    std::shared_ptr<const Assertion> SchemaBuilder::buildAssertion(const Element& definition,
        const SchemaSource& source) const
    {
        std::optional<std::string> test = definition.getAttribute("test");
        xpath::XPathContext context = namespacesInScope(definition);
        std::string defaultNamespace = defaultElementNamespace(definition, source);

        if (!test.has_value())
            fail(definition, i18n::format(i18n::Message::SCHEMA_ASSERT_NO_TEST, {definition.getLocalName()}));
        if (!defaultNamespace.empty())
            context.namespaces[""] = defaultNamespace;
        try
        {
            xpath::XPathExpression expression = xpath::detail::ExpressionAccess::compileAssertion(test.value(), context);
            return std::make_shared<const Assertion>(Assertion{std::move(expression), std::move(context)});
        }
        catch (const DomException& error)
        {
            fail(definition, i18n::format(i18n::Message::SCHEMA_ASSERT_INVALID, {test.value(), error.getMessage()}));
        }
    }

    // Утверждения xs:assert тела типа или xs:extension / xs:restriction — после унаследованных
    // The xs:assert assertions of a type body or of xs:extension / xs:restriction — after the inherited ones
    void SchemaBuilder::readAssertions(const Element& body, const SchemaSource& source, ComplexType& type) const
    {
        for (const Element* child : schemaChildren(body))
        {
            if (isSchemaElement(*child, "assert"))
                type.assertions.push_back(buildAssertion(*child, source));
        }
    }

    /*
    ** Фасеты xs:assertion шага ограничения простого типа. У списка $value —
    ** последовательность значений, которой в модели «Берёсты» нет, поэтому
    ** такое утверждение — ошибка загрузки (честнее, чем неверная проверка).
    **
    ** The xs:assertion facets of a restriction step of a simple type. For a list $value is
    ** a sequence of values, which the Beresta model does not have, so
    ** such an assertion is a loading error (more honest than a wrong check).
    */
    void SchemaBuilder::readAssertionFacets(SimpleType& type, const Element& restriction,
        const SchemaSource& source) const
    {
        for (const Element* child : schemaChildren(restriction))
        {
            if (!isSchemaElement(*child, "assertion"))
                continue;
            if (type.variety == Variety::LIST)
                fail(*child, i18n::format(i18n::Message::SCHEMA_ASSERTION_LIST));
            type.facets.assertions.push_back(buildAssertion(*child, source));
        }
    }
}
