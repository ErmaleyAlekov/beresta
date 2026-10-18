#include "parser/parse_session.hpp"
#include "i18n/i18n.hpp"
#include "parser/internal/declaration_reader.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::detail
{
    /*
    ** XML-декларация распознаётся только в самом начале документа:
    ** "<?xml" и сразу пробельный символ ("<?xml-stylesheet" — обычная PI).
    ** Псевдо-атрибуты идут строго в порядке version, encoding, standalone.
    **
    ** The XML declaration is recognized only at the very start of the document:
    ** "<?xml" immediately followed by a whitespace character ("<?xml-stylesheet" is an ordinary PI).
    ** The pseudo-attributes go strictly in the order version, encoding, standalone.
    */
    void ParseSession::parseXmlDeclaration()
    {
        XmlScanner& scanner = _inputs.main();
        std::string_view next = scanner.slice(5, 6);

        if (!scanner.startsWith("<?xml") || next.empty() || !utils::isWhitespace(next[0]))
            return;
        scanner.advance(5);
        XmlDeclaration declaration;
        if (!readPseudoAttribute("version", declaration.version))
            scanner.fail(i18n::format(i18n::Message::DECLARATION_VERSION_REQUIRED));
        readPseudoAttribute("encoding", declaration.encoding);
        readStandalone(declaration);
        scanner.skipWhitespace();
        scanner.expect("?>");
        validateXmlDeclaration(declaration);
        _standalone = declaration.standalone.value_or(false);
        _handler.xmlDeclaration(declaration);
    }

    // Псевдо-атрибуты XML-декларации читает общий помощник (он же — для текстовой декларации)
    // The pseudo-attributes of the XML declaration are read by a common helper (the same one as for the text declaration)
    bool ParseSession::readPseudoAttribute(std::string_view name, std::string& value)
    {
        return detail::readPseudoAttribute(_inputs.main(), name, value);
    }

    // standalone допускает только "yes" и "no"
    // standalone allows only "yes" and "no"
    void ParseSession::readStandalone(XmlDeclaration& declaration)
    {
        std::string value;

        if (!readPseudoAttribute("standalone", value))
            return;
        if (value == "yes")
            declaration.standalone = true;
        else if (value == "no")
            declaration.standalone = false;
        else
            _inputs.main().fail(i18n::format(i18n::Message::STANDALONE_VALUE));
    }
}
