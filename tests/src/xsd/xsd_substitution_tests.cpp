#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"
#include "i18n/i18n.hpp"

#include <string>

namespace
{
    /*
    ** Группа замены по мотивам случаев W3C XSD Test Suite (msData/element,
    ** sunData/ElemDecl/substGroupAffiliation): абстрактная голова
    ** publication; book — расширение её типа; magazine берёт тип головы;
    ** novel — член группы book, а значит транзитивно и publication;
    ** draft абстрактный и в документе стоять не может. pair — модель
    ** «последовательность разных имён», которая без групп шла бы жадным путём.
    **
    ** A substitution group after cases of the W3C XSD Test Suite (msData/element,
    ** sunData/ElemDecl/substGroupAffiliation): the abstract head
    ** publication; book is an extension of its type; magazine takes the head's type;
    ** novel is a member of the book group, and hence transitively of publication;
    ** draft is abstract and cannot stand in a document. pair is a model
    ** "a sequence of different names" that without groups would take the greedy path.
    */
    const char* const SUBSTITUTION_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xmlns:s=\"urn:s\" targetNamespace=\"urn:s\""
        " elementFormDefault=\"qualified\">"
        "<xs:complexType name=\"Publication\"><xs:sequence><xs:element name=\"title\" type=\"xs:string\"/>"
        "</xs:sequence></xs:complexType>"
        "<xs:complexType name=\"Book\"><xs:complexContent><xs:extension base=\"s:Publication\"><xs:sequence>"
        "<xs:element name=\"isbn\" type=\"xs:string\"/></xs:sequence></xs:extension></xs:complexContent>"
        "</xs:complexType>"
        "<xs:element name=\"publication\" type=\"s:Publication\" abstract=\"true\"/>"
        "<xs:element name=\"book\" type=\"s:Book\" substitutionGroup=\"s:publication\"/>"
        "<xs:element name=\"magazine\" substitutionGroup=\"s:publication\"/>"
        "<xs:element name=\"novel\" substitutionGroup=\"s:book\"/>"
        "<xs:element name=\"draft\" type=\"s:Book\" substitutionGroup=\"s:publication\" abstract=\"true\"/>"
        "<xs:element name=\"shelf\"><xs:complexType><xs:sequence>"
        "<xs:element ref=\"s:publication\" maxOccurs=\"unbounded\"/>"
        "<xs:element name=\"label\" type=\"xs:string\" minOccurs=\"0\"/></xs:sequence></xs:complexType></xs:element>"
        "<xs:element name=\"pair\"><xs:complexType><xs:sequence><xs:element ref=\"s:book\"/>"
        "<xs:element name=\"end\" type=\"xs:string\"/></xs:sequence></xs:complexType></xs:element>"
        "</xs:schema>";

    // Ошибки документа в пространстве urn:s (одинаковые во всех трёх режимах)
    // The errors of a document in the urn:s namespace (equal in all three modes)
    std::vector<beresta::Diagnostic> errorsFor(const std::string& xsd, const std::string& root,
        const std::string& body)
    {
        return support::agreedSchemaErrors(xsd, "<s:" + root + " xmlns:s=\"urn:s\">" + body + "</s:" + root + ">");
    }

    /*
    ** Члены группы стоят на месте головы — прямые, с типом головы и
    ** транзитивные; голова-ссылка в «жадной» модели тоже принимает
    ** заменителя. Содержимое проверяется по объявлению самого заменителя:
    ** у book обязателен isbn, у magazine его быть не может.
    **
    ** Group members stand in place of the head — direct ones, ones with the head's type
    ** and transitive ones; a head reference in a "greedy" model also accepts
    ** a substitute. The content is checked by the declaration of the substitute itself:
    ** book requires isbn, magazine cannot have it.
    */
    void testMembersReplaceHead()
    {
        std::string book = "<s:book><s:title>A</s:title><s:isbn>1</s:isbn></s:book>";

        TEST_ASSERT(errorsFor(SUBSTITUTION_XSD, "shelf", book + "<s:magazine><s:title>M</s:title></s:magazine>"
            "<s:novel><s:title>N</s:title><s:isbn>2</s:isbn></s:novel><s:label>x</s:label>").empty());
        TEST_ASSERT(errorsFor(SUBSTITUTION_XSD, "pair", "<s:novel><s:title>N</s:title><s:isbn>2</s:isbn>"
            "</s:novel><s:end>e</s:end>").empty());
        std::vector<beresta::Diagnostic> errors = errorsFor(SUBSTITUTION_XSD, "shelf",
            "<s:book><s:title>A</s:title></s:book><s:magazine><s:title>M</s:title><s:isbn>1</s:isbn></s:magazine>");
        TEST_ASSERT(errors.size() == 2);
        TEST_ASSERT(errors[0].message.find("/s:shelf/s:book") == 0);
        TEST_ASSERT(errors[1].message.find("/s:shelf/s:magazine") == 0);
    }

    /*
    ** Не заменители: абстрактная голова (сообщение об abstract), абстрактный
    ** член draft и посторонний элемент (оба — ошибка модели), голова
    ** book на месте publication допустима, но не наоборот.
    **
    ** Not substitutes: the abstract head (the abstract message), the abstract
    ** member draft and an unrelated element (both are model errors), the head
    ** book in place of publication is allowed, but not the other way round.
    */
    void testNonMembersRejected()
    {
        std::vector<beresta::Diagnostic> errors = errorsFor(SUBSTITUTION_XSD, "shelf",
            "<s:publication><s:title>P</s:title></s:publication>");
        std::string abstractText = beresta::i18n::format(beresta::i18n::Message::XSD_ABSTRACT_ELEMENT);

        TEST_ASSERT(errors.size() == 1);
        TEST_ASSERT(errors[0].message.find(abstractText) != std::string::npos);
        TEST_ASSERT(errorsFor(SUBSTITUTION_XSD, "shelf",
            "<s:draft><s:title>D</s:title><s:isbn>1</s:isbn></s:draft>").size() == 1);
        TEST_ASSERT(errorsFor(SUBSTITUTION_XSD, "shelf", "<s:label>x</s:label>").size() == 1);
        TEST_ASSERT(errorsFor(SUBSTITUTION_XSD, "pair", "<s:magazine><s:title>M</s:title></s:magazine>"
            "<s:end>e</s:end>").size() == 1);
    }

    /*
    ** block головы: "substitution" закрывает группу целиком, "extension" —
    ** только члены с расширенным типом (magazine с типом головы остаётся).
    ** final головы и кольцо групп — ошибки самой схемы (e-props-correct.4).
    **
    ** The head's block: "substitution" closes the group entirely, "extension" —
    ** only the members with an extended type (magazine with the head's type stays).
    ** The head's final and a ring of groups are errors of the schema itself (e-props-correct.4).
    */
    void testBlockAndFinal()
    {
        std::string blocked(SUBSTITUTION_XSD);
        std::string book = "<s:book><s:title>A</s:title><s:isbn>1</s:isbn></s:book>";
        std::string magazine = "<s:magazine><s:title>M</s:title></s:magazine>";
        std::string head = "type=\"s:Publication\" abstract=\"true\"";

        blocked.replace(blocked.find(head), head.size(), "type=\"s:Publication\" block=\"extension\"");
        TEST_ASSERT(errorsFor(blocked, "shelf", book).size() == 1);
        TEST_ASSERT(errorsFor(blocked, "shelf", magazine).empty());
        blocked.replace(blocked.find("block=\"extension\""), 17, "block=\"substitution\"");
        TEST_ASSERT(errorsFor(blocked, "shelf", magazine).size() == 1);
        std::string closed(SUBSTITUTION_XSD);
        closed.replace(closed.find(head), head.size(), "type=\"s:Publication\" final=\"extension\"");
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(closed), beresta::DomException);
        std::string ring(SUBSTITUTION_XSD);
        ring.replace(ring.find(head), head.size(), "type=\"s:Publication\" substitutionGroup=\"s:magazine\"");
        TEST_ASSERT_THROWS(beresta::xsd::Schema::fromString(ring), beresta::DomException);
    }
}

namespace suites
{
    // Регистрация тестов групп замены XSD
    // Registers the XSD substitution group tests
    void registerXsdSubstitutionTests(test::TestRunner& runner)
    {
        runner.add("XSD: группы замены — члены вместо головы", testMembersReplaceHead);
        runner.add("XSD: группы замены — абстрактные и посторонние", testNonMembersRejected);
        runner.add("XSD: группы замены — block и final", testBlockAndFinal);
    }
}
