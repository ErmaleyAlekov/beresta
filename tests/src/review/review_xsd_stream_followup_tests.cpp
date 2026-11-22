#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"
#include "xsd/internal/validation_budget.hpp"

namespace
{
    // Вложенное утверждение сохраняет отмену пространства имён со своего предка.
    // A nested assertion preserves the namespace undeclaration inherited from its ancestor.
    void testNestedAssertionNamespace()
    {
        const std::string schema =
            "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" targetNamespace=\"urn:a\">"
            "<xs:element name=\"outer\"><xs:complexType><xs:sequence>"
            "<xs:element name=\"mid\"><xs:complexType><xs:sequence>"
            "<xs:element name=\"inner\"><xs:complexType><xs:assert test=\"true()\"/>"
            "</xs:complexType></xs:element></xs:sequence></xs:complexType></xs:element></xs:sequence>"
            "<xs:assert test=\"exists(mid/inner)\" xpathDefaultNamespace=\"##local\"/>"
            "</xs:complexType></xs:element></xs:schema>";
        const std::string xml =
            "<outer xmlns=\"urn:a\">"
            "<mid xmlns=\"\">"
            "<inner/>"
            "</mid>"
            "</outer>";
        TEST_ASSERT(support::agreedSchemaErrors(schema, xml).empty());
    }

    // Превышение бюджета оставляет расход прежним.
    // Exceeding the budget leaves the amount used unchanged.
    void testChargeValidationBudget()
    {
        std::size_t used = beresta::xsd::detail::VALIDATION_BUDGET - 1;
        TEST_ASSERT_THROWS(beresta::xsd::detail::chargeValidation(used, 2, "resource"), beresta::DomException);
        TEST_ASSERT(used == beresta::xsd::detail::VALIDATION_BUDGET - 1);
    }
}

namespace suites
{
    // Регистрирует проверки потоковой валидации XSD.
    // Registers tests for streaming XSD validation.
    void registerReviewXsdStreamFollowupTests(test::TestRunner& runner)
    {
        runner.add("XSD: пространство имён вложенного утверждения", testNestedAssertionNamespace);
        runner.add("XSD: превышение бюджета валидации", testChargeValidationBudget);
    }
}
