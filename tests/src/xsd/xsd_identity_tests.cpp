#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "support/parse_helpers.hpp"

namespace
{
    /*
    ** Магазин в пространстве имён urn:shop: ключ товара по артикулу sku
    ** (decimal — сравнение по значению), уникальный необязательный ean и
    ** ссылки заказов на товары. Префикс s: в xpath разрешается по xmlns:s
    ** документа схемы.
    **
    ** A shop in the urn:shop namespace: a product key by the sku article
    ** (decimal — comparison by value), a unique optional ean and
    ** references from orders to products. The s: prefix in xpath is resolved by the xmlns:s
    ** of the schema document.
    */
    const char* const SHOP_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xmlns:s=\"urn:shop\""
        " targetNamespace=\"urn:shop\" elementFormDefault=\"qualified\">"
        "<xs:element name=\"shop\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"product\" maxOccurs=\"unbounded\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"sku\" type=\"xs:decimal\"/><xs:element name=\"ean\" type=\"xs:string\" minOccurs=\"0\"/>"
        "</xs:sequence></xs:complexType></xs:element>"
        "<xs:element name=\"order\" minOccurs=\"0\" maxOccurs=\"unbounded\"><xs:complexType>"
        "<xs:attribute name=\"sku\" type=\"xs:decimal\" use=\"required\"/></xs:complexType></xs:element>"
        "</xs:sequence></xs:complexType>"
        "<xs:key name=\"productKey\"><xs:selector xpath=\"s:product\"/><xs:field xpath=\"s:sku\"/></xs:key>"
        "<xs:unique name=\"eanUnique\"><xs:selector xpath=\"s:product\"/><xs:field xpath=\"s:ean\"/></xs:unique>"
        "<xs:keyref name=\"orderRef\" refer=\"s:productKey\"><xs:selector xpath=\"s:order\"/>"
        "<xs:field xpath=\"@sku\"/></xs:keyref></xs:element></xs:schema>";

    // Ошибки проверки магазина с товарами и заказами
    // The validation errors of a shop with products and orders
    std::vector<beresta::Diagnostic> shopErrors(const std::string& body)
    {
        return support::schemaErrors(SHOP_XSD, "<shop xmlns=\"urn:shop\">" + body + "</shop>");
    }

    /*
    ** xs:key, xs:unique, xs:keyref: корректный документ (1.0 = 1 и 2.50 = 2.5
    ** по значению decimal, товары без ean не мешают уникальности), затем по
    ** одному нарушению: повтор ключа (1 и 01), повтор ean, ссылка в никуда.
    **
    ** xs:key, xs:unique, xs:keyref: a valid document (1.0 = 1 and 2.50 = 2.5
    ** by the decimal value, products without ean do not break uniqueness), then
    ** one violation each: a repeated key (1 and 01), a repeated ean, a dangling reference.
    */
    void testKeyUniqueKeyref()
    {
        std::string products = "<product><sku>1</sku><ean>A</ean></product><product><sku>2.5</sku></product>"
            "<product><sku>3</sku></product>";

        for (const beresta::Diagnostic& error : shopErrors(products + "<order sku=\"1.0\"/><order sku=\"2.50\"/>"))
            TEST_ASSERT_EQ(error.message, "");
        std::vector<beresta::Diagnostic> duplicate = shopErrors(products + "<product><sku>01</sku></product>");
        TEST_ASSERT(duplicate.size() == 1 && duplicate[0].message.find("xs:key \"productKey\"") != std::string::npos);
        TEST_ASSERT(shopErrors(products + "<product><sku>4</sku><ean>A</ean></product>").size() == 1);
        std::vector<beresta::Diagnostic> dangling = shopErrors(products + "<order sku=\"7\"/>");
        TEST_ASSERT(dangling.size() == 1 && dangling[0].message.find("/shop/order") == 0);
    }

    /*
    ** Составной ключ из двух атрибутов на любой глубине (.//): разные пары
    ** допустимы, повтор пары — ошибка, отсутствие поля ключа — ошибка,
    ** поле, выбирающее несколько узлов, — ошибка.
    **
    ** A composite key of two attributes at any depth (.//): different pairs
    ** are allowed, a repeated pair is an error, a missing key field is an error,
    ** a field selecting several nodes is an error.
    */
    void testCompositeKeyRules()
    {
        std::string xsd = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"><xs:element name=\"r\">"
            "<xs:complexType><xs:sequence><xs:any processContents=\"skip\" maxOccurs=\"unbounded\"/></xs:sequence>"
            "</xs:complexType><xs:key name=\"pair\"><xs:selector xpath=\".//item\"/><xs:field xpath=\"@a\"/>"
            "<xs:field xpath=\"@b\"/></xs:key><xs:unique name=\"single\"><xs:selector xpath=\"box\"/>"
            "<xs:field xpath=\"*\"/></xs:unique></xs:element></xs:schema>";

        TEST_ASSERT(support::schemaErrors(xsd, "<r><item a=\"1\" b=\"x\"/><g><item a=\"1\" b=\"y\"/></g></r>").empty());
        TEST_ASSERT(support::schemaErrors(xsd, "<r><item a=\"1\" b=\"x\"/><g><item a=\"1\" b=\"x\"/></g></r>").size() == 1);
        TEST_ASSERT(support::schemaErrors(xsd, "<r><item a=\"1\"/></r>").size() == 1);
        TEST_ASSERT(support::schemaErrors(xsd, "<r><box><p/><q/></box></r>").size() == 1);
    }
}

namespace suites
{
    // Регистрация тестов ограничений идентичности XSD
    // Registers the XSD identity constraint tests
    void registerXsdIdentityTests(test::TestRunner& runner)
    {
        runner.add("XSD: xs:key, xs:unique, xs:keyref", testKeyUniqueKeyref);
        runner.add("XSD: составной ключ, обязательные поля, одно значение поля", testCompositeKeyRules);
    }
}
