#include "suites/test_suites.hpp"
#include "support/parse_helpers.hpp"
#include "i18n/i18n.hpp"
#include "xsd/regex/xsd_regex.hpp"

namespace
{
    /*
    ** Проверка обычного допустимого пустого xs:sequence: найти глобальный тип t
    ** через lookup API из заголовка и проверить ContentType::EMPTY_CONTENT и hasParticle false.
    **
    ** Checking a normal valid empty xs:sequence: find the global type t
    ** through the lookup API from the header and check ContentType::EMPTY_CONTENT and hasParticle false.
    */
    void testEmptySequence()
    {
        auto schema = support::schemaFromBody("<xs:complexType name=\"t\"><xs:sequence/></xs:complexType>");
        bool found = false;
        const auto* type = schema->findType("", "t", found).complex;
        TEST_ASSERT(found);
        TEST_ASSERT(type != nullptr);
        TEST_ASSERT(type->content == beresta::xsd::ContentType::EMPTY_CONTENT);
        TEST_ASSERT(!type->hasParticle);
    }

    /*
    ** Проверка xs:all с minOccurs=0 и maxOccurs=1 допустима.
    **
    ** Checking xs:all with minOccurs=0 and maxOccurs=1 is allowed.
    */
    void testAllWithZeroOne()
    {
        TEST_ASSERT(support::schemaFromBody("<xs:complexType name=\"t\">"
            "<xs:all minOccurs=\"0\" maxOccurs=\"1\"><xs:element name=\"e\"/></xs:all>"
            "</xs:complexType>") != nullptr);
        try
        {
            support::schemaFromBody("<xs:complexType name=\"t\">"
                "<xs:all minOccurs=\"2\" maxOccurs=\"1\"><xs:element name=\"e\"/></xs:all>"
                "</xs:complexType>");
        }
        catch (const beresta::DomException& error)
        {
            TEST_ASSERT(error.getCode() == beresta::DomErrorCode::SCHEMA_ERROR);
            TEST_ASSERT(error.getMessage().find(beresta::i18n::format(beresta::i18n::Message::SCHEMA_ALL_OCCURS))
                != std::string::npos);
            return;
        }
        TEST_ASSERT(false);
    }

    /*
    ** Простой xs:element с minOccurs=+1 допускается.
    **
    ** A simple xs:element with minOccurs=+1 is allowed.
    */
    void testSimpleElementWithOne()
    {
        TEST_ASSERT(support::schemaFromBody("<xs:complexType name=\"t\"><xs:sequence>"
            "<xs:element name=\"e\" type=\"xs:string\" minOccurs=\"+1\"/>"
            "</xs:sequence></xs:complexType>") != nullptr);
    }

    /*
    ** Проверка обычного класса диапазона Regex [a-c] и вычитания [a-z-[aeiou]].
    **
    ** Checking a normal range Regex [a-c] and subtraction [a-z-[aeiou]].
    */
    void testRegexRangeAndSubtraction()
    {
        beresta::xsd::Regex range("([a-c]|z)*[a-c]");
        beresta::xsd::Regex subtraction("[a-z-[aeiou]]");

        TEST_ASSERT(range.matches("a") && range.matches("b") && range.matches("c"));
        TEST_ASSERT(range.matches("zabc") && !range.matches("az"));
        TEST_ASSERT(!range.matches("d"));

        TEST_ASSERT(subtraction.matches("b") && subtraction.matches("c") && subtraction.matches("d"));
        TEST_ASSERT(!subtraction.matches("a") && !subtraction.matches("e") && !subtraction.matches("i"));
        try
        {
            const beresta::xsd::Regex missingPropertyEnd(R"(\p{Lu{2})");
        }
        catch (const beresta::DomException& error)
        {
            TEST_ASSERT(error.getCode() == beresta::DomErrorCode::SCHEMA_ERROR);
            TEST_ASSERT(error.getMessage().find(beresta::i18n::format(beresta::i18n::Message::PATTERN_PROPERTY_SYNTAX))
                != std::string::npos);
            return;
        }
        TEST_ASSERT(false);
    }
}

namespace suites
{
    // Регистрация тестов проверки XSD
    // Registers the tests of XSD validation
    void registerReviewXsdTests(test::TestRunner& runner)
    {
        runner.add("XSD: пустой xs:sequence", testEmptySequence);
        runner.add("XSD: xs:all с minOccurs=0 и maxOccurs=1", testAllWithZeroOne);
        runner.add("XSD: простой xs:element с minOccurs=+1", testSimpleElementWithOne);
        runner.add("XSD: классы диапазона и вычитания в Regex", testRegexRangeAndSubtraction);
    }
}
