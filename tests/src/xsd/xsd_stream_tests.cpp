#include "suites/test_suites.hpp"
#include "support/recording_handler.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Схема заказа, задевающая все этапы проверки: ID/IDREF, перечисление,
    ** choice, fixed, xsi:nil, xs:all, вложенный счётчик (последовательность
    ** до трёх пар c — отложенная проверка модели) и xs:any strict.
    **
    ** An order schema touching all validation phases: ID/IDREF, an enumeration,
    ** choice, fixed, xsi:nil, xs:all, a nested counter (a sequence of
    ** up to three pairs of c — a deferred model check) and xs:any strict.
    */
    const char* const ORDER_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"><xs:element name=\"order\"><xs:complexType>"
        "<xs:sequence><xs:element name=\"line\" maxOccurs=\"unbounded\" nillable=\"true\"><xs:complexType>"
        "<xs:sequence><xs:element name=\"sku\" type=\"xs:ID\"/><xs:choice>"
        "<xs:element name=\"qty\" type=\"xs:positiveInteger\"/><xs:element name=\"weight\" type=\"xs:decimal\"/>"
        "</xs:choice><xs:element name=\"note\" type=\"xs:string\" minOccurs=\"0\" fixed=\"ok\"/></xs:sequence>"
        "<xs:attribute name=\"ref\" type=\"xs:IDREF\"/><xs:attribute name=\"kind\" use=\"required\">"
        "<xs:simpleType><xs:restriction base=\"xs:token\"><xs:enumeration value=\"a\"/>"
        "<xs:enumeration value=\"b\"/></xs:restriction></xs:simpleType></xs:attribute></xs:complexType></xs:element>"
        "<xs:element name=\"meta\" minOccurs=\"0\"><xs:complexType><xs:all><xs:element name=\"x\" type=\"xs:int\"/>"
        "<xs:element name=\"y\" type=\"xs:int\" minOccurs=\"0\"/></xs:all></xs:complexType></xs:element>"
        "<xs:element name=\"grid\" minOccurs=\"0\"><xs:complexType><xs:sequence maxOccurs=\"3\">"
        "<xs:element name=\"c\" type=\"xs:int\" minOccurs=\"2\" maxOccurs=\"2\"/></xs:sequence></xs:complexType>"
        "</xs:element><xs:any processContents=\"strict\" minOccurs=\"0\"/></xs:sequence></xs:complexType>"
        "</xs:element><xs:element name=\"extra\" type=\"xs:boolean\"/></xs:schema>";

    // Документ с ошибками на разных строках и у одноимённых соседей
    // A document with errors on different lines and at same-named siblings
    const char* const ORDER_XML =
        "<order xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n"
        "  <line kind=\"a\"><sku>s1</sku><qty>2</qty></line>\n"
        "  <line kind=\"c\" ref=\"s9\"><sku>s1</sku><qty>0</qty><note>bad</note></line>\n"
        "  <line kind=\"b\" xsi:nil=\"true\"><sku>s2</sku></line>\n"
        "  <line kind=\"a\">text<sku>s3</sku><weight>x</weight><qty>1</qty></line>\n"
        "  <line><sku>s4</sku></line>\n"
        "  <meta><y>1</y><y>2</y></meta>\n"
        "  <grid><c>1</c><c>2</c><c>3</c></grid>\n"
        "  <extra>maybe</extra>\n"
        "</order>\n";

    /*
    ** SAX без дерева, разбор в DOM и проверка готового дерева дают один и
    ** тот же список: тексты, пути с номерами соседей, строки и порядок.
    **
    ** SAX without a tree, parsing into a DOM and validation of a ready tree give one and
    ** the same list: the texts, the paths with sibling numbers, the lines and the order.
    */
    void testModesAgree()
    {
        support::SchemaRuns runs = support::runSchemaModes(ORDER_XSD, ORDER_XML);
        bool secondLine = false;

        TEST_ASSERT_EQ(support::describeDiagnostics(runs.sax), support::describeDiagnostics(runs.parsed));
        TEST_ASSERT(runs.sax.size() >= 12);
        for (const beresta::Diagnostic& error : runs.tree)
            secondLine = secondLine || (error.message.find("/order/line[2]: ") == 0 && error.position.line == 3);
        TEST_ASSERT(secondLine);
        TEST_ASSERT(support::sameDiagnostics(runs.sax, runs.tree));
    }

    /*
    ** Валидатор — декоратор: обработчик получает те же события, что и без
    ** схемы. Без ErrorHandler ошибки схемы бросаются после разбора, когда
    ** все события уже доставлены.
    **
    ** The validator is a decorator: the handler gets the same events as without
    ** a schema. Without an ErrorHandler schema errors are thrown after parsing, when
    ** all events have already been delivered.
    */
    void testEventsPassThrough()
    {
        support::RecordingHandler plain;
        support::RecordingHandler validated;
        support::RecordingHandler strict;
        beresta::CollectingErrorHandler errors;
        beresta::ParserOptions options;

        beresta::SaxParser().parse(ORDER_XML, plain);
        options.schema = beresta::xsd::Schema::fromString(ORDER_XSD);
        options.errorHandler = &errors;
        beresta::SaxParser(options).parse(ORDER_XML, validated);
        TEST_ASSERT_EQ(validated.getLog(), plain.getLog());
        TEST_ASSERT(!errors.getDiagnostics().empty());
        options.errorHandler = nullptr;
        TEST_ASSERT_THROWS(beresta::SaxParser(options).parse(ORDER_XML, strict), beresta::ParseException);
        TEST_ASSERT_EQ(strict.getLog(), plain.getLog());
    }

    /*
    ** Глубокий документ: проверка идёт без рекурсии, а путь ошибки у самого
    ** нижнего элемента собирается из всех трёх тысяч шагов.
    **
    ** A deep document: the validation goes without recursion, and the error path at the
    ** lowest element is assembled from all three thousand steps.
    */
    void testDeepDocument()
    {
        const std::string xsd = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"><xs:element name=\"n\">"
            "<xs:complexType><xs:choice><xs:element ref=\"n\"/><xs:element name=\"v\" type=\"xs:int\"/></xs:choice>"
            "</xs:complexType></xs:element></xs:schema>";
        const std::size_t depth = 3000;
        std::string xml;
        std::string path;
        beresta::ParserOptions options;

        for (std::size_t level = 0; level < depth; ++level)
        {
            xml += "<n>";
            path += "/n";
        }
        xml += "<v>x</v>";
        for (std::size_t level = 0; level < depth; ++level)
            xml += "</n>";
        options.maxDepth = depth + 10;
        support::SchemaRuns runs = support::runSchemaModes(xsd, xml, options);
        TEST_ASSERT(runs.sax.size() == 1);
        TEST_ASSERT(support::sameDiagnostics(runs.sax, runs.parsed));
        TEST_ASSERT(support::sameDiagnostics(runs.sax, runs.tree));
        TEST_ASSERT(runs.tree.front().message.compare(0, path.size() + 3, path + "/v:") == 0);
    }

    /*
    ** Парсер без пространств имён: пары «URI + имя» вычисляет валидатор по
    ** объявлениям xmlns — ошибки те же, что и при обычном разборе.
    **
    ** A parser without namespaces: the validator computes the "URI + name" pairs by the
    ** xmlns declarations — the errors are the same as with ordinary parsing.
    */
    void testNamespacesResolvedByValidator()
    {
        const std::string xsd = "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" targetNamespace=\"urn:q\""
            " xmlns:q=\"urn:q\" elementFormDefault=\"qualified\"><xs:element name=\"r\"><xs:complexType><xs:sequence>"
            "<xs:element name=\"v\" type=\"xs:QName\" maxOccurs=\"unbounded\"/></xs:sequence>"
            "<xs:attribute name=\"n\" type=\"xs:int\"/></xs:complexType></xs:element></xs:schema>";
        const std::string xml = "<p:r xmlns:p=\"urn:q\" n=\"x\"><p:v>p:a</p:v><p:v>z:a</p:v><v xmlns=\"urn:q\">b</v>"
            "<w/></p:r>";
        beresta::ParserOptions plain;
        support::SchemaRuns aware = support::runSchemaModes(xsd, xml);

        plain.namespaceAware = false;
        support::SchemaRuns unaware = support::runSchemaModes(xsd, xml, plain);
        TEST_ASSERT(aware.sax.size() == 3);
        TEST_ASSERT_EQ(support::describeDiagnostics(unaware.sax), support::describeDiagnostics(aware.sax));
        TEST_ASSERT_EQ(support::describeDiagnostics(unaware.parsed), support::describeDiagnostics(aware.parsed));
        TEST_ASSERT(support::sameDiagnostics(aware.sax, aware.tree));
    }
}

namespace suites
{
    // Регистрация тестов потоковой проверки по схеме
    // Registers the tests of streaming schema validation
    void registerXsdStreamTests(test::TestRunner& runner)
    {
        runner.add("XSD поток: SAX, DOM и дерево дают одинаковые ошибки", testModesAgree);
        runner.add("XSD поток: события проходят сквозь валидатор", testEventsPassThrough);
        runner.add("XSD поток: глубокий документ", testDeepDocument);
        runner.add("XSD поток: пространства имён без поддержки парсера", testNamespacesResolvedByValidator);
    }
}
