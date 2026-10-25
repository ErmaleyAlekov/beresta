#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"
#include "i18n/i18n.hpp"

#include <string>

namespace
{
    /*
    ** Тесты для рекурсивной модели содержимого с наследованием (паттерн Composite)
    ** Схема COMPOSITE_XSD: глобальные элементы Box type=BoxType и Panel type=PanelType;
    ** complexType BoxType = sequence из <xs:element ref=Panel minOccurs=0 maxOccurs=unbounded/> + атрибут id типа xs:ID;
    ** complexType PanelType = complexContent/extension base=BoxType с атрибутом title типа xs:string.
    ** Вторая схема RENAMED_XSD — та же, но элемент Panel переименован в APanel (и ref=APanel).
    **
    ** Tests for recursive content model with inheritance (Composite pattern)
    ** COMPOSITE_XSD schema: global elements Box type=BoxType and Panel type=PanelType;
    ** complexType BoxType = sequence of <xs:element ref=Panel minOccurs=0 maxOccurs=unbounded/> + id attribute of type xs:ID;
    ** complexType PanelType = complexContent/extension base=BoxType with title attribute of type xs:string.
    ** Second schema RENAMED_XSD is the same, but element Panel renamed to APanel (and ref=APanel).
    */
    const char* const COMPOSITE_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"Box\" type=\"BoxType\"/>"
        "<xs:element name=\"Panel\" type=\"PanelType\"/>"
        "<xs:complexType name=\"BoxType\">"
        "<xs:sequence>"
        "<xs:element ref=\"Panel\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "</xs:sequence>"
        "<xs:attribute name=\"id\" type=\"xs:ID\"/>"
        "</xs:complexType>"
        "<xs:complexType name=\"PanelType\">"
        "<xs:complexContent>"
        "<xs:extension base=\"BoxType\">"
        "<xs:attribute name=\"title\" type=\"xs:string\"/>"
        "</xs:extension>"
        "</xs:complexContent>"
        "</xs:complexType>"
        "</xs:schema>";

    const char* const RENAMED_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"Box\" type=\"BoxType\"/>"
        "<xs:element name=\"APanel\" type=\"PanelType\"/>"
        "<xs:complexType name=\"BoxType\">"
        "<xs:sequence>"
        "<xs:element ref=\"APanel\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "</xs:sequence>"
        "<xs:attribute name=\"id\" type=\"xs:ID\"/>"
        "</xs:complexType>"
        "<xs:complexType name=\"PanelType\">"
        "<xs:complexContent>"
        "<xs:extension base=\"BoxType\">"
        "<xs:attribute name=\"title\" type=\"xs:string\"/>"
        "</xs:extension>"
        "</xs:complexContent>"
        "</xs:complexType>"
        "</xs:schema>";

    /*
    ** Проверка корректности рекурсивной модели содержимого
    ** Checks the correctness of the recursive content model
    */
    void testRecursiveContentModel()
    {
        TEST_ASSERT(support::agreedSchemaErrors(COMPOSITE_XSD, "<Box><Panel title=\"a\"><Panel/></Panel></Box>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(RENAMED_XSD, "<Box><APanel title=\"a\"><APanel/></APanel></Box>").empty());
    }

    /*
    ** Проверка ограничений на вложенность типов
    ** Checks the constraints on type nesting
    */
    void testTypeNestingConstraints()
    {
        TEST_ASSERT(!support::agreedSchemaErrors(COMPOSITE_XSD, "<Box><Panel><Box/></Panel></Box>").empty());
    }

    /*
    ** Проверка обнаружения циклов в наследовании
    ** Checks the detection of cycles in inheritance
    */
    void testInheritanceCycles()
    {
        const char* const CYCLE_XSD_1 =
            "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
            "<xs:complexType name=\"A\">"
            "<xs:complexContent>"
            "<xs:extension base=\"A\"/>"
            "</xs:complexContent>"
            "</xs:complexType>"
            "</xs:schema>";

        const char* const CYCLE_XSD_2 =
            "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
            "<xs:complexType name=\"A\">"
            "<xs:complexContent>"
            "<xs:extension base=\"B\"/>"
            "</xs:complexContent>"
            "</xs:complexType>"
            "<xs:complexType name=\"B\">"
            "<xs:complexContent>"
            "<xs:extension base=\"A\"/>"
            "</xs:complexContent>"
            "</xs:complexType>"
            "</xs:schema>";

        const char* const CHAIN_XSD =
            "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
            "<xs:complexType name=\"A\">"
            "<xs:complexContent>"
            "<xs:extension base=\"B\"/>"
            "</xs:complexContent>"
            "</xs:complexType>"
            "<xs:complexType name=\"B\">"
            "<xs:sequence>"
            "<xs:element name=\"c\" type=\"C\"/>"
            "</xs:sequence>"
            "</xs:complexType>"
            "<xs:complexType name=\"C\">"
            "<xs:complexContent>"
            "<xs:extension base=\"A\"/>"
            "</xs:complexContent>"
            "</xs:complexType>"
            "</xs:schema>";

        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(CYCLE_XSD_1), beresta::DomException);
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(CYCLE_XSD_2), beresta::DomException);
        TEST_ASSERT(beresta::xsd::Schema::fromString(CHAIN_XSD) != nullptr);
        // Тип X вне цикла наследует цикл A / B: ошибка — именно о цикле
        // The type X outside the cycle derives from the A / B cycle: the error is about the cycle
        std::string outsider = CYCLE_XSD_2;
        outsider.insert(outsider.find('>') + 1,
            "<xs:element name=\"e\" type=\"X\"/><xs:complexType name=\"X\"><xs:complexContent>"
            "<xs:extension base=\"A\"/></xs:complexContent></xs:complexType>");
        std::string message;
        try
        {
            beresta::xsd::Schema::fromString(outsider);
        }
        catch (const beresta::DomException& error)
        {
            message = error.what();
        }
        TEST_ASSERT(message.find(beresta::i18n::format(beresta::i18n::Message::SCHEMA_CYCLIC_DERIVATION))
            != std::string::npos);
    }
}

namespace suites
{
    // Регистрация тестов для рекурсивной модели содержимого с наследованием
    // Registers the tests for recursive content model with inheritance
    void registerXsdRecursiveDerivationTests(test::TestRunner& runner)
    {
        runner.add("XSD: recursive content model", testRecursiveContentModel);
        runner.add("XSD: type nesting constraints", testTypeNestingConstraints);
        runner.add("XSD: inheritance cycles", testInheritanceCycles);
    }
}
