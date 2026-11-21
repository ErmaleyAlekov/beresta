#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"
#include "encoding/encoding_detector.hpp"
#include "i18n/i18n.hpp"
#include "parser/internal/declaration_reader.hpp"
#include "parser/internal/expansion_budget.hpp"

#include <limits>

/*
** Регрессии ревью кода 2026-11-21 (задачи Т-2…Т-4): кодировка из
** декларации, переполнение бюджета раскрытий, сообщения о декларациях,
** линейная склейка текста.
**
** Regressions of the code review of 2026-11-21 (tasks T-2…T-4): the encoding from
** the declaration, the overflow of the expansion budget, the messages about declarations,
** linear text concatenation.
*/
namespace
{
    // Разбор (или текстовая декларация) падает с сообщением каталога с этим идентификатором
    // Parsing (or a text declaration) fails with the catalog message with this identifier
    bool failsWith(const char* input, bool textDeclaration, beresta::i18n::Message message)
    {
        try
        {
            if (textDeclaration)
                beresta::detail::stripTextDeclaration(input);
            else
                beresta::XmlParser().parse(input);
        }
        catch (const beresta::DomException& error)
        {
            return std::string(error.what()).find(beresta::i18n::format(message)) != std::string::npos;
        }
        return false;
    }

    /*
    ** "badencoding" — не encoding (Т-2). Сумма в бюджете не переполняется даже
    ** при лимите около SIZE_MAX (Т-3).
    **
    ** "badencoding" is not encoding (T-2). The sum in the budget does not overflow even
    ** with a limit near SIZE_MAX (T-3).
    */
    void testEncodingAndBudget()
    {
        beresta::ParserOptions options;
        options.maxExpandedSize = std::numeric_limits<std::size_t>::max() - 1;
        beresta::detail::ExpansionBudget budget(options);

        TEST_ASSERT_EQ(beresta::encoding::readDeclaredEncoding(
            "<?xml version=\"1.0\" badencoding=\"cp1251\" encoding=\"utf-8\"?><a/>"), std::string("utf-8"));
        TEST_ASSERT_EQ(beresta::encoding::readDeclaredEncoding("<?xml version=\"1.0\" xencoding=\"cp1251\"?>"),
            std::string());
        budget.charge(5, 1);
        TEST_ASSERT_THROWS(budget.charge(std::numeric_limits<std::size_t>::max() - 2, 1), beresta::DomException);
    }

    /*
    ** Чужой псевдоатрибут в текстовой декларации — ошибка порядка, а не «нет encoding» (Т-3);
    ** <?xml?> и <?XML ...?> в начале документа — испорченная декларация, а не «не на месте» (Т-4).
    **
    ** A foreign pseudo-attribute in a text declaration is an order error, not "no encoding" (T-3);
    ** <?xml?> and <?XML ...?> at the start of the document are a broken declaration, not "out of place" (T-4).
    */
    void testDeclarationMessages()
    {
        using beresta::i18n::Message;

        TEST_ASSERT(failsWith("<?xml foo=\"1\" encoding=\"UTF-8\"?>x", true, Message::TEXT_DECLARATION_ORDER));
        TEST_ASSERT(failsWith("<?xml version=\"1.0\"?>x", true, Message::TEXT_DECLARATION_ENCODING));
        TEST_ASSERT(failsWith("<?xml?><a/>", false, Message::XML_DECLARATION_MALFORMED));
        TEST_ASSERT(failsWith("<?XML version=\"1.0\"?><a/>", false, Message::XML_DECLARATION_MALFORMED));
        TEST_ASSERT(failsWith("<a/><?xml version=\"1.0\"?>", false, Message::XML_DECLARATION_PLACEMENT));
    }

    /*
    ** 100 000 секций CDATA при coalesceCData — один текстовый узел без квадратичной
    ** склейки (Т-4); дописывание проверяет запреты на стыке кусков.
    **
    ** 100 000 CDATA sections with coalesceCData are one text node without quadratic
    ** concatenation (T-4); appending checks the bans at the joint of the pieces.
    */
    void testLinearTextAppend()
    {
        std::string input = "<a>";
        beresta::ParserOptions options;
        options.coalesceCData = true;
        beresta::Comment comment("a");
        beresta::CDataSection section("x]");

        for (int index = 0; index < 100000; ++index)
            input += "<![CDATA[x]]>";
        beresta::Document document = beresta::XmlParser(options).parse(input + "</a>");
        TEST_ASSERT(document.getRoot()->getTextContent().size() == 100000);
        TEST_ASSERT(document.getRoot()->getChildCount() == 1);
        TEST_ASSERT_THROWS(comment.appendValue("-"), beresta::DomException);
        comment.appendValue("-b");
        TEST_ASSERT_THROWS(comment.appendValue("-"), beresta::DomException);
        TEST_ASSERT_THROWS(section.appendValue("]>"), beresta::DomException);
        TEST_ASSERT_EQ(comment.getValue(), std::string("a-b"));
    }
}

namespace suites
{
    // Регистрация регрессий ревью: разбор
    // Registers the review regressions: parsing
    void registerReviewParserTests(test::TestRunner& runner)
    {
        runner.add("Ревью Т-2/Т-3: encoding из декларации, бюджет раскрытий", testEncodingAndBudget);
        runner.add("Ревью Т-3/Т-4: сообщения о декларациях", testDeclarationMessages);
        runner.add("Ревью Т-4: линейная склейка текста", testLinearTextAppend);
    }
}
