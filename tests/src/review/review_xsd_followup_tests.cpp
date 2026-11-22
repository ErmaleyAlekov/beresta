#include "suites/test_suites.hpp"
#include "support/parse_helpers.hpp"
#include "support/schema_helpers.hpp"
#include "i18n/i18n.hpp"
#include "xsd/internal/validation_budget.hpp"

namespace
{
    // Проверяет диагностику обязательного атрибута фасета.
    // Checks the diagnostic for a required facet attribute.
    void checkSchemaError(const std::string& body, const std::string& expected)
    {
        try
        {
            support::schemaFromBody(body);
        }
        catch (const beresta::DomException& error)
        {
            TEST_ASSERT(error.getCode() == beresta::DomErrorCode::SCHEMA_ERROR);
            TEST_ASSERT(error.getMessage().find(expected) != std::string::npos);
            return;
        }
        TEST_ASSERT(false);
    }

    // У pattern и minInclusive атрибут value обязателен.
    // Both pattern and minInclusive require the value attribute.
    void testMissingFacetValue()
    {
        std::string message = beresta::i18n::format(beresta::i18n::Message::SCHEMA_REQUIRED_ATTRIBUTE, {"value"});
        checkSchemaError("<xs:simpleType name=\"t\"><xs:restriction base=\"xs:string\">"
            "<xs:pattern/></xs:restriction></xs:simpleType>", message);
        checkSchemaError("<xs:simpleType name=\"t\"><xs:restriction base=\"xs:int\">"
            "<xs:minInclusive/></xs:restriction></xs:simpleType>", message);
    }

    // Вложенный простой тип задаёт тип текста при ограничении.
    // The nested simple type determines the text type of the restriction.
    void testSimpleContentRestriction()
    {
        const std::string schema =
            "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
            "<xs:complexType name=\"base\"><xs:simpleContent><xs:extension base=\"xs:decimal\"/>"
            "</xs:simpleContent></xs:complexType>"
            "<xs:complexType name=\"t\"><xs:simpleContent><xs:restriction base=\"base\">"
            "<xs:simpleType><xs:restriction base=\"xs:int\"/></xs:simpleType>"
            "</xs:restriction></xs:simpleContent></xs:complexType>"
            "<xs:element name=\"n\" type=\"t\"/></xs:schema>";
        TEST_ASSERT(support::agreedSchemaErrors(schema, "<n>7</n>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(schema, "<n>1.5</n>").size() == 1);
    }

    // Смешанный тип со значением по умолчанию должен допускать пустое содержимое.
    // A mixed type with a default value must allow empty content.
    void testMixedTypeDefault()
    {
        std::string prefix = "<xs:complexType name=\"t\" mixed=\"true\"><xs:sequence>"
            "<xs:element name=\"e\" type=\"xs:string\"";
        std::string suffix = "/></xs:sequence></xs:complexType><xs:element name=\"root\" type=\"t\" default=\"value\"/>";
        std::string message = beresta::i18n::format(beresta::i18n::Message::SCHEMA_VALUE_CONSTRAINT_CONTENT,
            {"root", "default"});
        checkSchemaError(prefix + suffix, message);
        TEST_ASSERT(support::schemaFromBody(prefix + " minOccurs=\"0\"" + suffix) != nullptr);
    }

    // Проверяет бюджет валидации.
    // Checks the validation budget.
    void testValidationBudget()
    {
        std::size_t used = beresta::xsd::detail::VALIDATION_BUDGET - 2;
        beresta::xsd::detail::chargeValidation(used, 1, "resource1");
        TEST_ASSERT(used == beresta::xsd::detail::VALIDATION_BUDGET - 1);
        beresta::xsd::detail::chargeValidation(used, 1, "resource2");
        TEST_ASSERT(used == beresta::xsd::detail::VALIDATION_BUDGET);
    }
}

namespace suites
{
    // Регистрирует тесты загрузки и проверки схемы.
    // Registers schema loading and validation tests.
    void registerReviewXsdFollowupTests(test::TestRunner& runner)
    {
        runner.add("XSD: обязательное значение фасета", testMissingFacetValue);
        runner.add("XSD: simpleContent restriction с вложенным xs:int", testSimpleContentRestriction);
        runner.add("XSD: значение по умолчанию для смешанного типа", testMixedTypeDefault);
        runner.add("XSD: бюджет валидации", testValidationBudget);
    }
}
