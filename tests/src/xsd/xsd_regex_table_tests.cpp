#include "suites/test_suites.hpp"
#include "exception/dom_exception.hpp"
#include "xsd/regex/unicode_tables.hpp"
#include <string_view>
#include <cstring> // for std::strcmp

namespace
{
    /*
    ** Проверяет инварианты таблицы символов Unicode.
    ** Проверяет, что в каждом элементе first <= last, элементы идут по возрастанию,
    ** не пересекаются (first[i] > last[i-1]), last <= 0x10FFFF, major — одна из букв "LMNPZSC".
    **
    ** Checks the invariants of the Unicode character tables.
    ** Ensures that in each element first <= last, elements are in ascending order,
    ** do not overlap (first[i] > last[i-1]), last <= 0x10FFFF, and major is one of the letters "LMNPZSC".
    */
    void testUnicodeCategories()
    {
        std::string_view validMajors = "LMNPZSC";
        for (std::size_t i = 0; i < beresta::xsd::UNICODE_CATEGORY_COUNT; ++i)
        {
            TEST_ASSERT(beresta::xsd::UNICODE_CATEGORIES[i].first <= beresta::xsd::UNICODE_CATEGORIES[i].last);
            TEST_ASSERT(beresta::xsd::UNICODE_CATEGORIES[i].last <= 0x10FFFF);
            TEST_ASSERT(validMajors.find(beresta::xsd::UNICODE_CATEGORIES[i].major) != std::string_view::npos);
            if (i > 0)
            {
                TEST_ASSERT(beresta::xsd::UNICODE_CATEGORIES[i].first > beresta::xsd::UNICODE_CATEGORIES[i - 1].last);
            }
        }
    }

    /*
    ** Проверяет инварианты таблицы блоков Unicode.
    ** Проверяет, что в каждом элементе first <= last, имя начинается с "Is",
    ** ровно один блок с именем IsBasicLatin и он 0x0000-0x007F.
    **
    ** Checks the invariants of the Unicode block tables.
    ** Ensures that in each element first <= last, the name starts with "Is",
    ** and there is exactly one block named IsBasicLatin which is 0x0000-0x007F.
    */
    void testUnicodeBlocks()
    {
        bool foundBasicLatin = false;
        for (std::size_t i = 0; i < beresta::xsd::UNICODE_BLOCK_COUNT; ++i)
        {
            TEST_ASSERT(beresta::xsd::UNICODE_BLOCKS[i].first <= beresta::xsd::UNICODE_BLOCKS[i].last);
            TEST_ASSERT(beresta::xsd::UNICODE_BLOCKS[i].name[0] == 'I' && beresta::xsd::UNICODE_BLOCKS[i].name[1] == 's');
            if (std::strcmp(beresta::xsd::UNICODE_BLOCKS[i].name, "IsBasicLatin") == 0)
            {
                TEST_ASSERT(!foundBasicLatin);
                TEST_ASSERT(beresta::xsd::UNICODE_BLOCKS[i].first == 0x0000 && beresta::xsd::UNICODE_BLOCKS[i].last == 0x007F);
                foundBasicLatin = true;
            }
        }
        TEST_ASSERT(foundBasicLatin);
    }

    /*
    ** Проверяет инварианты таблицы XML_NAME_START_RANGES и XML_NAME_EXTRA_RANGES.
    ** Проверяет, что по возрастанию, не пересекаются, ни один код из EXTRA не попадает в диапазоны START.
    **
    ** Checks the invariants of the XML_NAME_START_RANGES and XML_NAME_EXTRA_RANGES tables.
    ** Ensures that they are in ascending order, do not overlap, and no code from EXTRA is in the START ranges.
    */
    void testXmlNameRanges()
    {
        for (std::size_t i = 0; i < beresta::xsd::XML_NAME_START_COUNT; ++i)
        {
            TEST_ASSERT(beresta::xsd::XML_NAME_START_RANGES[i].first <= beresta::xsd::XML_NAME_START_RANGES[i].last);
            if (i > 0)
            {
                TEST_ASSERT(beresta::xsd::XML_NAME_START_RANGES[i].first > beresta::xsd::XML_NAME_START_RANGES[i - 1].last);
            }
        }

        for (std::size_t i = 0; i < beresta::xsd::XML_NAME_EXTRA_COUNT; ++i)
        {
            TEST_ASSERT(beresta::xsd::XML_NAME_EXTRA_RANGES[i].first <= beresta::xsd::XML_NAME_EXTRA_RANGES[i].last);
            if (i > 0)
            {
                TEST_ASSERT(beresta::xsd::XML_NAME_EXTRA_RANGES[i].first > beresta::xsd::XML_NAME_EXTRA_RANGES[i - 1].last);
            }
        }

        for (std::size_t i = 0; i < beresta::xsd::XML_NAME_EXTRA_COUNT; ++i)
        {
            for (std::size_t j = 0; j < beresta::xsd::XML_NAME_START_COUNT; ++j)
            {
                TEST_ASSERT(!(beresta::xsd::XML_NAME_EXTRA_RANGES[i].first <= beresta::xsd::XML_NAME_START_RANGES[j].last && beresta::xsd::XML_NAME_EXTRA_RANGES[i].last >= beresta::xsd::XML_NAME_START_RANGES[j].first));
            }
        }
    }
}

namespace suites
{
    // Регистрация тестов инвариантов таблиц символов Unicode
    // Registers the tests of the Unicode character table invariants
    void registerXsdRegexTableTests(test::TestRunner& runner)
    {
        runner.add("XSD regex: инварианты таблицы символов Unicode", testUnicodeCategories);
        runner.add("XSD regex: инварианты таблицы блоков Unicode", testUnicodeBlocks);
        runner.add("XSD regex: инварианты таблицы XML_NAME_START_RANGES и XML_NAME_EXTRA_RANGES", testXmlNameRanges);
    }
}
