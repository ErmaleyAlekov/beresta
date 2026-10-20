#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"

#include <string>

namespace
{
    const char* const XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"root\">"
        "   <xs:complexType>"
        "       <xs:sequence>"
        "           <xs:element name=\"e\" maxOccurs=\"unbounded\">"
        "               <xs:complexType>"
        "                   <xs:attribute name=\"att\" type=\"xs:string\" default=\"a\"/>"
        "               </xs:complexType>"
        "           </xs:element>"
        "       </xs:sequence>"
        "   </xs:complexType>"
        "   <xs:unique name=\"u\">"
        "       <xs:selector xpath=\".//e\"/>"
        "       <xs:field xpath=\"@att\"/>"
        "   </xs:unique>"
        "</xs:element>"
        "</xs:schema>";

    void testDefaultAttributes()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<root><e/><e/></root>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<root><e att=\"a\"/><e/></root>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD, "<root><e att=\"b\"/><e/></root>").empty());
    }

    const char* const XSD_FIXED_ATTRIBUTE_KEY =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"root\">"
        "   <xs:complexType>"
        "       <xs:sequence>"
        "           <xs:element name=\"e\" maxOccurs=\"unbounded\">"
        "               <xs:complexType>"
        "                   <xs:attribute name=\"k\" type=\"xs:int\" fixed=\"7\"/>"
        "               </xs:complexType>"
        "           </xs:element>"
        "       </xs:sequence>"
        "   </xs:complexType>"
        "   <xs:key name=\"k\">"
        "       <xs:selector xpath=\"e\"/>"
        "       <xs:field xpath=\"@k\"/>"
        "   </xs:key>"
        "</xs:element>"
        "</xs:schema>";

    void testFixedAttributeKey()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD_FIXED_ATTRIBUTE_KEY, "<root><e/></root>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD_FIXED_ATTRIBUTE_KEY, "<root><e/><e k=\"07\"/></root>").size() == 1);
    }

    const char* const XSD_DEFAULT_ATTRIBUTE_ON_CHILD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"root\">"
        "   <xs:complexType>"
        "       <xs:sequence>"
        "           <xs:element name=\"e\" maxOccurs=\"unbounded\">"
        "               <xs:complexType>"
        "                   <xs:sequence>"
        "                       <xs:element name=\"c\">"
        "                           <xs:complexType>"
        "                               <xs:attribute name=\"att\" type=\"xs:string\" default=\"z\"/>"
        "                           </xs:complexType>"
        "                       </xs:element>"
        "                   </xs:sequence>"
        "               </xs:complexType>"
        "           </xs:element>"
        "       </xs:sequence>"
        "   </xs:complexType>"
        "   <xs:unique name=\"u\">"
        "       <xs:selector xpath=\"e\"/>"
        "       <xs:field xpath=\"c/@att\"/>"
        "   </xs:unique>"
        "</xs:element>"
        "</xs:schema>";

    void testDefaultAttributeOnChild()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD_DEFAULT_ATTRIBUTE_ON_CHILD, "<root><e><c/></e><e><c/></e></root>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD_DEFAULT_ATTRIBUTE_ON_CHILD, "<root><e><c att=\"y\"/></e><e><c/></e></root>").empty());
    }

    const char* const XSD_COMPLEX_FIELD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"root\">"
        "   <xs:complexType>"
        "       <xs:sequence>"
        "           <xs:element name=\"uid\" maxOccurs=\"unbounded\">"
        "               <xs:complexType>"
        "                   <xs:sequence>"
        "                       <xs:element name=\"pid\">"
        "                           <xs:complexType>"
        "                               <xs:attribute name=\"p\" type=\"xs:string\"/>"
        "                           </xs:complexType>"
        "                       </xs:element>"
        "                   </xs:sequence>"
        "               </xs:complexType>"
        "           </xs:element>"
        "       </xs:sequence>"
        "   </xs:complexType>"
        "   <xs:unique name=\"u\">"
        "       <xs:selector xpath=\"uid\"/>"
        "       <xs:field xpath=\"pid\"/>"
        "   </xs:unique>"
        "</xs:element>"
        "<xs:element name=\"root2\">"
        "   <xs:complexType>"
        "       <xs:sequence>"
        "           <xs:element name=\"sc\" maxOccurs=\"unbounded\">"
        "               <xs:complexType>"
        "                   <xs:simpleContent>"
        "                       <xs:extension base=\"xs:int\">"
        "                           <xs:attribute name=\"lang\" type=\"xs:string\"/>"
        "                       </xs:extension>"
        "                   </xs:simpleContent>"
        "               </xs:complexType>"
        "           </xs:element>"
        "       </xs:sequence>"
        "   </xs:complexType>"
        "   <xs:unique name=\"u2\">"
        "       <xs:selector xpath=\"sc\"/>"
        "       <xs:field xpath=\".\"/>"
        "   </xs:unique>"
        "</xs:element>"
        "</xs:schema>";

    void testComplexField()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD_COMPLEX_FIELD, "<root><uid><pid p=\"1\"/></uid></root>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD_COMPLEX_FIELD, "<root2><sc>1</sc><sc lang=\"x\">01</sc></root2>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD_COMPLEX_FIELD, "<root2><sc>1</sc><sc>2</sc></root2>").empty());
    }

    const char* const XSD_UNTYPED_ATTRIBUTE =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"foo\">"
        "   <xs:complexType>"
        "       <xs:anyAttribute namespace=\"##any\" processContents=\"skip\"/>"
        "   </xs:complexType>"
        "   <xs:unique name=\"u\">"
        "       <xs:selector xpath=\".\"/>"
        "       <xs:field xpath=\"@bar\"/>"
        "   </xs:unique>"
        "</xs:element>"
        "</xs:schema>";

    void testUntypedAttribute()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD_UNTYPED_ATTRIBUTE, "<foo bar=\"abc\"/>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD_UNTYPED_ATTRIBUTE, "<foo other=\"abc\"/>").empty());
    }

    const char* const XSD_QNAME_KEYS =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:element name=\"root\">"
        "   <xs:complexType>"
        "       <xs:choice maxOccurs=\"unbounded\">"
        "           <xs:element name=\"key\" type=\"xs:QName\"/>"
        "           <xs:element name=\"ref\" type=\"xs:QName\"/>"
        "       </xs:choice>"
        "   </xs:complexType>"
        "   <xs:key name=\"key\">"
        "       <xs:selector xpath=\"key\"/>"
        "       <xs:field xpath=\".\"/>"
        "   </xs:key>"
        "   <xs:keyref name=\"kr\" refer=\"key\">"
        "       <xs:selector xpath=\"ref\"/>"
        "       <xs:field xpath=\".\"/>"
        "   </xs:keyref>"
        "</xs:element>"
        "</xs:schema>";

    void testQNameKeys()
    {
        TEST_ASSERT(support::agreedSchemaErrors(XSD_QNAME_KEYS, "<root xmlns:p=\"urn:a\" xmlns:q=\"urn:a\"><key>p:abc</key><key>q:abc</key></root>").size() == 1);
        TEST_ASSERT(support::agreedSchemaErrors(XSD_QNAME_KEYS, "<root xmlns:p=\"urn:a\" xmlns:q=\"urn:b\"><key>p:abc</key><key>q:abc</key></root>").empty());
        TEST_ASSERT(support::agreedSchemaErrors(XSD_QNAME_KEYS, "<root xmlns:p=\"urn:a\" xmlns:q=\"urn:a\"><key>p:abc</key><ref>q:abc</ref></root>").empty());
    }
}

namespace suites
{
    // Регистрация тестов полей ограничений идентичности
    // Registers the tests of identity field constraints
    void registerXsdIdentityFieldTests(test::TestRunner& runner)
    {
        runner.add("XSD ключи: тесты полей ограничений идентичности", testDefaultAttributes);
        runner.add("XSD ключи: тесты ключа с фиксированным атрибутом", testFixedAttributeKey);
        runner.add("XSD ключи: тесты умолчательного атрибута на дочернем элементе", testDefaultAttributeOnChild);
        runner.add("XSD ключи: тесты сложного поля", testComplexField);
        runner.add("XSD ключи: тесты неотипизированного атрибута", testUntypedAttribute);
        runner.add("XSD ключи: тесты QName ключей", testQNameKeys);
    }
}
