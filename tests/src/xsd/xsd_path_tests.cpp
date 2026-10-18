#include "suites/test_suites.hpp"
#include "support/parse_helpers.hpp"

#include <string>

namespace
{
    /*
    ** Схема групп: g повторяется, в нём v (int) повторяется и w (int)
    ** необязателен; у g есть ID и IDREF, а значения w уникальны в r.
    ** Пути в сообщениях строятся лениво (только при ошибке), и эти тесты
    ** сверяют их с тем, что давало построение путей заранее.
    **
    ** The group schema: g repeats, inside it v (int) repeats and w (int)
    ** is optional; g has an ID and an IDREF, and the values of w are unique within r.
    ** Paths in messages are built lazily (only on an error), and these tests
    ** compare them with what building the paths in advance gave.
    */
    const char* const GROUPS_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"r\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"g\" maxOccurs=\"unbounded\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"v\" type=\"xs:int\" maxOccurs=\"unbounded\"/>"
        "<xs:element name=\"w\" type=\"xs:int\" minOccurs=\"0\"/></xs:sequence>"
        "<xs:attribute name=\"id\" type=\"xs:ID\"/><xs:attribute name=\"ref\" type=\"xs:IDREF\"/>"
        "</xs:complexType></xs:element></xs:sequence></xs:complexType>"
        "<xs:unique name=\"uw\"><xs:selector xpath=\"g\"/><xs:field xpath=\"w\"/></xs:unique>"
        "</xs:element></xs:schema>";

    // Схема значений с разными правилами пробелов и фасетом длины
    // A value schema with different whitespace rules and a length facet
    const char* const VALUES_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"x\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"t\"><xs:simpleType><xs:restriction base=\"xs:token\">"
        "<xs:enumeration value=\"a b\"/></xs:restriction></xs:simpleType></xs:element>"
        "<xs:element name=\"d\" type=\"xs:decimal\"/><xs:element name=\"n\" type=\"xs:NCName\"/>"
        "<xs:element name=\"s\"><xs:simpleType><xs:restriction base=\"xs:string\">"
        "<xs:length value=\"3\"/></xs:restriction></xs:simpleType></xs:element>"
        "</xs:sequence></xs:complexType></xs:element></xs:schema>";

    // Число ошибок, сообщение которых начинается с пути path и двоеточия
    // The number of errors whose message starts with the path path and a colon
    std::size_t errorsAt(const std::vector<beresta::Diagnostic>& errors, const std::string& path)
    {
        std::size_t count = 0;

        for (const beresta::Diagnostic& error : errors)
        {
            if (error.message.compare(0, path.size() + 1, path + ":") == 0)
                ++count;
        }
        return count;
    }

    // Номер среди одноимённых соседей ставится только при повторе имени; >8 детей — другая ветка
    // The number among same-named siblings is set only when the name repeats; >8 children is another branch
    void testErrorPaths()
    {
        std::vector<beresta::Diagnostic> errors = support::schemaErrors(GROUPS_XSD,
            "<r><g><v>1</v></g><g><v>x</v><v>2</v><v>y</v></g></r>");
        std::string many = "<r><g>";

        TEST_ASSERT(errors.size() == 2);
        TEST_ASSERT(errorsAt(errors, "/r/g[2]/v[1]") == 1);
        TEST_ASSERT(errorsAt(errors, "/r/g[2]/v[3]") == 1);
        for (int index = 0; index < 9; ++index)
            many += "<v>1</v>";
        errors = support::schemaErrors(GROUPS_XSD, many + "<v>z</v><w>3</w></g></r>");
        TEST_ASSERT(errors.size() == 1);
        TEST_ASSERT(errorsAt(errors, "/r/g/v[10]") == 1);
        errors = support::schemaErrors(GROUPS_XSD, "<r><g><w>1</w><v>2</v></g></r>");
        TEST_ASSERT(errors.size() == 1);
        TEST_ASSERT(errorsAt(errors, "/r/g/w") == 1);
    }

    // Пути ошибок ID/IDREF (после обхода) и xs:unique (элемент найден через XPath)
    // The paths of ID/IDREF errors (after the traversal) and xs:unique (the element is found through XPath)
    void testIdentityPaths()
    {
        std::vector<beresta::Diagnostic> errors = support::schemaErrors(GROUPS_XSD,
            "<r><g id=\"a\"><v>1</v></g><g id=\"a\" ref=\"b\"><v>1</v></g></r>");

        TEST_ASSERT(errors.size() == 2);
        TEST_ASSERT(errorsAt(errors, "/r/g[2]") == 2);
        errors = support::schemaErrors(GROUPS_XSD,
            "<r><g><v>1</v><w>5</w></g><g><v>1</v></g><g><v>1</v><w>5</w></g></r>");
        TEST_ASSERT(errors.size() == 1);
        TEST_ASSERT(errorsAt(errors, "/r/g[3]") == 1);
    }

    // Правила пробелов: token и decimal сжимаются, string — нет; текст с комментарием внутри
    // Whitespace rules: token and decimal are collapsed, string is not; a text with a comment inside
    void testWhitespaceRules()
    {
        std::vector<beresta::Diagnostic> errors = support::schemaErrors(VALUES_XSD,
            "<x><t>  a \t b \n</t><d> 1.5 </d><n>ab</n><s> z </s></x>");

        for (const beresta::Diagnostic& error : errors)
            TEST_ASSERT_EQ(error.message, "");
        TEST_ASSERT(errors.empty());
        errors = support::schemaErrors(VALUES_XSD, "<x><t>a  c</t><d>1<!--c-->.5</d><n>a b</n><s>zz</s></x>");
        TEST_ASSERT(errors.size() == 3);
        TEST_ASSERT(errorsAt(errors, "/x/t") == 1);
        TEST_ASSERT(errorsAt(errors, "/x/n") == 1);
        TEST_ASSERT(errorsAt(errors, "/x/s") == 1);
    }
}

namespace suites
{
    // Регистрация тестов путей в сообщениях и правил пробелов XSD
    // Registers the tests of paths in messages and XSD whitespace rules
    void registerXsdPathTests(test::TestRunner& runner)
    {
        runner.add("XSD: пути в сообщениях об ошибках", testErrorPaths);
        runner.add("XSD: пути ошибок ID, IDREF и xs:unique", testIdentityPaths);
        runner.add("XSD: правила пробелов и текст с комментарием", testWhitespaceRules);
    }
}
