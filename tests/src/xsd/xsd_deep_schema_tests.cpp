#include "suites/test_suites.hpp"
#include "beresta.hpp"
#include <cstddef>
#include <string>

namespace
{
    /*
    ** Вспомогательная функция для создания вложенных элементов.
    ** Helper function to create nested elements.
    */
    std::string nestedElements(std::size_t levels)
    {
        std::string schema = "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>";
        for (std::size_t i = 0; i < levels; ++i)
        {
            schema += "<xs:element name='e'><xs:complexType><xs:sequence>";
        }
        schema += "<xs:element name='leaf'/>";
        for (std::size_t i = 0; i < levels; ++i)
        {
            schema += "</xs:sequence></xs:complexType></xs:element>";
        }
        schema += "</xs:schema>";
        return schema;
    }

    /*
    ** Вспомогательная функция для создания вложенных простых типов.
    ** Helper function to create nested simple types.
    */
    std::string nestedSimpleTypes(std::size_t levels)
    {
        std::string schema = "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'><xs:simpleType name='t'>";
        for (std::size_t i = 0; i < levels; ++i)
        {
            schema += "<xs:restriction><xs:simpleType>";
        }
        schema += "<xs:restriction base='xs:string'/>";
        for (std::size_t i = 0; i < levels; ++i)
        {
            schema += "</xs:simpleType></xs:restriction>";
        }
        schema += "</xs:simpleType></xs:schema>";
        return schema;
    }

    /*
    ** Проверяет, отвергается ли схема как слишком глубокая: в тексте ошибки
    ** есть сработавший предел (256 — сборка схемы, 1024 — документ схемы).
    ** Checks if the schema is rejected as too deep: the error text contains
    ** the limit that fired (256 — building the schema, 1024 — the schema document).
    */
    bool rejectedAsTooDeep(const std::string& text, const std::string& limit)
    {
        beresta::ParserOptions options;
        options.maxDepth = static_cast<std::size_t>(-1);
        beresta::Document document = beresta::XmlParser(options).parse(text);
        try
        {
            beresta::xsd::Schema::fromDocuments({&document});
        }
        catch (const beresta::DomException& problem)
        {
            return std::string(problem.what()).find(limit) != std::string::npos;
        }
        return false;
    }

    /*
    ** Тест: вложенная схема из 30 уровней загружается без ошибок.
    ** Test: nested schema with 30 levels loads without errors.
    */
    void testNestedElements30()
    {
        std::string schemaText = nestedElements(30);
        beresta::ParserOptions options;
        options.maxDepth = static_cast<std::size_t>(-1);
        beresta::Document document = beresta::XmlParser(options).parse(schemaText);
        TEST_ASSERT(beresta::xsd::Schema::fromDocuments({&document}) != nullptr);
    }

    /*
    ** Тест: 100 вложенных локальных элементов с анонимными типами (400 уровней
    ** сборки) — ошибка схемы с пределом 256. Раньше предел частиц начинался
    ** заново на каждом анонимном типе, и такая цепочка росла до переполнения стека.
    ** Test: 100 nested local elements with anonymous types (400 building levels)
    ** — a schema error with the limit 256. Before, the particle limit started anew
    ** at every anonymous type, and such a chain grew until the stack overflowed.
    */
    void testNestedElements100()
    {
        TEST_ASSERT(rejectedAsTooDeep(nestedElements(100), "256"));
    }

    /*
    ** Тест: 5000 вложенных простых типов (10 000 уровней XML) отвергаются
    ** сразу, ещё до сборки, с пределом документа 1024 — без переполнения стека
    ** и без квадратичного обхода глубокого документа.
    ** Test: 5000 nested simple types (10,000 XML levels) are rejected at once,
    ** before building, with the document limit 1024 — without a stack overflow
    ** and without a quadratic traversal of the deep document.
    */
    void testNestedSimpleTypes5000()
    {
        TEST_ASSERT(rejectedAsTooDeep(nestedSimpleTypes(5000), "1024"));
    }
}

namespace suites
{
    // Регистрация тестов глубоких схем
    // Registers the deep schema tests
    void registerXsdDeepSchemaTests(test::TestRunner& runner)
    {
        runner.add("XSD: вложенная схема из 30 уровней загружается", testNestedElements30);
        runner.add("XSD: 100 вложенных анонимных типов — предел сборки 256", testNestedElements100);
        runner.add("XSD: документ схемы глубже 1024 отвергается до сборки", testNestedSimpleTypes5000);
    }
}
