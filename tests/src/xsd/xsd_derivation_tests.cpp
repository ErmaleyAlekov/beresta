#include "suites/test_suites.hpp"
#include "support/schema_helpers.hpp"
#include "i18n/i18n.hpp"

#include <string>

namespace
{
    /*
    ** Наследование для xsi:type по мотивам W3C XSD Test Suite
    ** (msData/element/elemZ*, sunData/ElemDecl/disallowedSubst): Address и
    ** его расширение UsAddress, ограничение ShortAddress, посторонний тип
    ** Other, абстрактный Shape и его расширение Circle; элементы с block
    ** на самом элементе и на типе; простые типы — встроенная цепочка
    ** xs:int -> xs:decimal и объединение.
    **
    ** Derivation for xsi:type after the W3C XSD Test Suite
    ** (msData/element/elemZ*, sunData/ElemDecl/disallowedSubst): Address and
    ** its extension UsAddress, the restriction ShortAddress, the unrelated type
    ** Other, the abstract Shape and its extension Circle; elements with block
    ** on the element itself and on the type; simple types — the built-in chain
    ** xs:int -> xs:decimal and a union.
    */
    const char* const DERIVATION_XSD =
        "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
        "<xs:complexType name=\"Address\"><xs:sequence><xs:element name=\"city\" type=\"xs:string\"/>"
        "<xs:element name=\"zip\" type=\"xs:string\" minOccurs=\"0\"/></xs:sequence></xs:complexType>"
        "<xs:complexType name=\"UsAddress\"><xs:complexContent><xs:extension base=\"Address\"><xs:sequence>"
        "<xs:element name=\"state\" type=\"xs:string\"/></xs:sequence></xs:extension></xs:complexContent>"
        "</xs:complexType>"
        "<xs:complexType name=\"ShortAddress\"><xs:complexContent><xs:restriction base=\"Address\"><xs:sequence>"
        "<xs:element name=\"city\" type=\"xs:string\"/></xs:sequence></xs:restriction></xs:complexContent>"
        "</xs:complexType>"
        "<xs:complexType name=\"Sealed\" block=\"restriction\"><xs:complexContent><xs:extension base=\"Address\"/>"
        "</xs:complexContent></xs:complexType>"
        "<xs:complexType name=\"SealedShort\"><xs:complexContent><xs:restriction base=\"Sealed\"><xs:sequence>"
        "<xs:element name=\"city\" type=\"xs:string\"/></xs:sequence></xs:restriction></xs:complexContent>"
        "</xs:complexType>"
        "<xs:complexType name=\"Other\"><xs:sequence><xs:element name=\"city\" type=\"xs:string\"/></xs:sequence>"
        "</xs:complexType>"
        "<xs:complexType name=\"Shape\" abstract=\"true\"><xs:attribute name=\"id\" type=\"xs:string\"/>"
        "</xs:complexType>"
        "<xs:complexType name=\"Circle\"><xs:complexContent><xs:extension base=\"Shape\">"
        "<xs:attribute name=\"r\" type=\"xs:decimal\" use=\"required\"/></xs:extension></xs:complexContent>"
        "</xs:complexType>"
        "<xs:simpleType name=\"Number\"><xs:union memberTypes=\"xs:int xs:boolean\"/></xs:simpleType>"
        "<xs:element name=\"root\"><xs:complexType><xs:sequence>"
        "<xs:element name=\"address\" type=\"Address\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "<xs:element name=\"fixed\" type=\"Address\" block=\"extension\" minOccurs=\"0\"/>"
        "<xs:element name=\"sealed\" type=\"Sealed\" minOccurs=\"0\"/>"
        "<xs:element name=\"shape\" type=\"Shape\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "<xs:element name=\"amount\" type=\"xs:decimal\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "<xs:element name=\"number\" type=\"Number\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "</xs:sequence></xs:complexType></xs:element></xs:schema>";

    // Ошибки документа <root> с объявленным префиксом xsi (одинаковые во всех режимах)
    // The errors of a <root> document with the xsi prefix declared (equal in all modes)
    std::vector<beresta::Diagnostic> errorsFor(const std::string& body)
    {
        return support::agreedSchemaErrors(DERIVATION_XSD,
            "<root xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">" + body + "</root>");
    }

    // Единственная ошибка, и в её тексте есть сообщение id с аргументом argument
    // The only error, and its text contains the message id with the argument argument
    bool onlyError(const std::vector<beresta::Diagnostic>& errors, beresta::i18n::Message id,
        std::string_view argument)
    {
        std::string expected = beresta::i18n::format(id, {argument});

        return errors.size() == 1 && errors[0].message.find(expected) != std::string::npos;
    }

    /*
    ** Допустимые xsi:type: расширение и ограничение объявленного типа
    ** (содержимое проверяется уже по ним), xs:int вместо xs:decimal, член
    ** объединения вместо объединения, производный тип вместо абстрактного.
    **
    ** Allowed xsi:type: an extension and a restriction of the declared type
    ** (the content is then checked by them), xs:int instead of xs:decimal, a member
    ** of a union instead of the union, a derived type instead of an abstract one.
    */
    void testDerivedTypesAccepted()
    {
        TEST_ASSERT(errorsFor("<address xsi:type=\"UsAddress\"><city>A</city><state>NY</state></address>"
            "<address xsi:type=\"ShortAddress\"><city>B</city></address>"
            "<shape xsi:type=\"Circle\" r=\"2\"/><amount xsi:type=\"xs:int\" xmlns:xs=\""
            "http://www.w3.org/2001/XMLSchema\">5</amount>"
            "<number xsi:type=\"xs:int\" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">7</number>").empty());
        TEST_ASSERT(errorsFor("<address xsi:type=\"UsAddress\"><city>A</city></address>").size() == 1);
        TEST_ASSERT(errorsFor("<amount xsi:type=\"xs:int\" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
            "1.5</amount>").size() == 1);
    }

    /*
    ** Недопустимые: посторонний тип, xs:string вместо xs:decimal, расширение
    ** при block="extension" у элемента, ограничение при block="restriction"
    ** у типа, абстрактный тип без xsi:type. Путь ошибки — к элементу.
    **
    ** Not allowed: an unrelated type, xs:string instead of xs:decimal, an extension
    ** with block="extension" on the element, a restriction with block="restriction"
    ** on the type, an abstract type without xsi:type. The error path leads to the element.
    */
    void testDerivationViolations()
    {
        using beresta::i18n::Message;
        std::string xs = " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"";
        std::vector<beresta::Diagnostic> errors = errorsFor("<address><city>A</city></address>"
            "<address xsi:type=\"Other\"><city>B</city></address>");

        TEST_ASSERT(onlyError(errors, Message::XSD_XSI_TYPE_NOT_DERIVED, "Other"));
        TEST_ASSERT(errors[0].message.find("/root/address[2]") == 0);
        TEST_ASSERT(onlyError(errorsFor("<amount xsi:type=\"xs:string\"" + xs + ">5</amount>"),
            Message::XSD_XSI_TYPE_NOT_DERIVED, "xs:string"));
        TEST_ASSERT(onlyError(errorsFor("<fixed xsi:type=\"UsAddress\"><city>A</city><state>S</state></fixed>"),
            Message::XSD_XSI_TYPE_NOT_DERIVED, "UsAddress"));
        TEST_ASSERT(errorsFor("<fixed xsi:type=\"ShortAddress\"><city>A</city></fixed>").empty());
        TEST_ASSERT(onlyError(errorsFor("<sealed xsi:type=\"SealedShort\"><city>A</city></sealed>"),
            Message::XSD_XSI_TYPE_NOT_DERIVED, "SealedShort"));
        TEST_ASSERT(onlyError(errorsFor("<shape id=\"1\"/>"), Message::XSD_ABSTRACT_TYPE, "Shape"));
    }
}

namespace suites
{
    // Регистрация тестов наследования для xsi:type: block и abstract
    // Registers the derivation tests for xsi:type: block and abstract
    void registerXsdDerivationTests(test::TestRunner& runner)
    {
        runner.add("XSD: xsi:type — производные типы", testDerivedTypesAccepted);
        runner.add("XSD: xsi:type — block и abstract", testDerivationViolations);
    }
}
