#include "parser/parse_session.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::detail
{
    /*
    ** XML-декларация распознаётся только в самом начале документа:
    ** "<?xml" и сразу пробельный символ ("<?xml-stylesheet" — обычная PI).
    ** Псевдо-атрибуты идут строго в порядке version, encoding, standalone.
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
            scanner.fail("в XML-декларации обязателен псевдо-атрибут version");
        readPseudoAttribute("encoding", declaration.encoding);
        readStandalone(declaration);
        scanner.skipWhitespace();
        scanner.expect("?>");
        validateXmlDeclaration(declaration);
        _handler.xmlDeclaration(declaration);
    }

    // name = "значение" или name = 'значение'; перед именем обязателен пробел
    bool ParseSession::readPseudoAttribute(std::string_view name, std::string& value)
    {
        XmlScanner& scanner = _inputs.main();
        std::size_t spaces = scanner.skipWhitespace();

        if (!scanner.startsWith(name))
            return false;
        if (spaces == 0)
            scanner.fail("ожидался пробел перед \"" + std::string(name) + "\"");
        scanner.advance(name.size());
        scanner.skipWhitespace();
        scanner.expect("=");
        scanner.skipWhitespace();
        char quote = scanner.peek();
        if (quote != '"' && quote != '\'')
            scanner.fail("значение в XML-декларации должно быть в кавычках");
        scanner.advance(1);
        value = std::string(scanner.readUntil(std::string_view(&quote, 1)));
        return true;
    }

    // standalone допускает только "yes" и "no"
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
            _inputs.main().fail("standalone может быть только \"yes\" или \"no\"");
    }
}
