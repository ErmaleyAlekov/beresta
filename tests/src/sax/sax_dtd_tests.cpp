#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

#include "beresta.hpp"

#include <string>

/*
** Раздел 2.6 XML 1.0: инструкции обработки обязаны передаваться
** приложению, даже если стоят внутри DTD. Раньше PI из DTD только
** проверялись и терялись; теперь SAX получает их, а DOM — нет.
**
** Section 2.6 of XML 1.0: processing instructions must be passed to
** the application even if they are inside a DTD. Previously PIs from a DTD were only
** checked and lost; now SAX receives them, and DOM does not.
*/
namespace
{
    // Записывает инструкции обработки строкой "цель:данные;" в порядке прихода
    // Records processing instructions as the string "target:data;" in the order of arrival
    class InstructionRecorder : public beresta::ContentHandler
    {
    public:
        // Обычная PI и PI из DTD записываются одинаково — так видно, что по умолчанию они не различаются
        // An ordinary PI and a PI from the DTD are recorded the same way — this shows they are not distinguished by default
        void processingInstruction(std::string_view target, std::string_view data) override
        {
            log += std::string(target) + ":" + std::string(data) + ";";
        }

        // Момент прихода DOCTYPE отмечается, чтобы проверить порядок событий
        // The moment the DOCTYPE arrives is marked to check the order of events
        void documentType(const beresta::DocumentType& documentType) override
        {
            log += "DOCTYPE " + documentType.getName() + ";";
        }

        std::string log;
    };

    // PI внутреннего и внешнего подмножества приходят в SAX до DOCTYPE и в порядке текста
    // PIs of the internal and external subsets arrive in SAX before the DOCTYPE and in text order
    void testSaxReceivesDtdInstructions()
    {
        std::filesystem::path root = support::makeTestDirectory("sax_dtd_pi");
        support::writeTestFile(root / "d.dtd", "<?ext из-внешнего?><!ELEMENT d ANY>");
        support::writeTestFile(root / "doc.xml",
            "<?top a?><!DOCTYPE d SYSTEM \"d.dtd\" [<?in b ?><!ENTITY % p \"<?pe c?>\"> %p;]><d><?body z?></d>");
        beresta::FileResourceResolver resolver(root);
        beresta::ParserOptions options;
        InstructionRecorder recorder;

        options.resolver = &resolver;
        options.loadExternalDtd = true;
        beresta::SaxParser(options).parseFile(root / "doc.xml", recorder);
        TEST_ASSERT_EQ(recorder.log, "top:a;in:b ;pe:c;ext:из-внешнего;DOCTYPE d;body:z;");
    }

    // В DOM PI из DTD не попадают, но текст внутреннего подмножества их сохраняет
    // PIs from the DTD do not go into the DOM, but the text of the internal subset keeps them
    void testDomSkipsDtdInstructions()
    {
        beresta::Document document = beresta::XmlParser().parse(
            "<?top a?><!DOCTYPE d [<?in b?>]><d><?body z?></d>");

        TEST_ASSERT(document.getChildCount() == 3);
        TEST_ASSERT(document.getChild(0)->getType() == beresta::NodeType::PROCESSING_INSTRUCTION);
        TEST_ASSERT(document.getChild(1)->getType() == beresta::NodeType::DOCUMENT_TYPE);
        TEST_ASSERT_EQ(document.getDocumentType()->getInternalSubset(), "<?in b?>");
        TEST_ASSERT_EQ(document.toString(), "<?top a?><!DOCTYPE d[<?in b?>]><d><?body z?></d>");
    }
}

namespace suites
{
    // Регистрация тестов PI из DTD
    // Registers the tests of PIs from the DTD
    void registerSaxDtdTests(test::TestRunner& runner)
    {
        runner.add("SAX: PI из DTD передаются приложению", testSaxReceivesDtdInstructions);
        runner.add("DOM: PI из DTD не становятся узлами", testDomSkipsDtdInstructions);
    }
}
