#include "parser/internal/declaration_reader.hpp"
#include "i18n/i18n.hpp"
#include "core/xml_declaration.hpp"
#include "exception/dom_exception.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::detail
{
    namespace
    {
        // Число пробельных символов от курсора — без сдвига курсора
        // The number of whitespace characters from the cursor — without moving the cursor
        std::size_t countWhitespaceAhead(const XmlScanner& scanner) noexcept
        {
            std::size_t offset = scanner.getOffset();
            std::size_t count = 0;
            std::string_view next = scanner.slice(offset, offset + 1);

            while (!next.empty() && utils::isWhitespace(next[0]))
            {
                ++count;
                next = scanner.slice(offset + count, offset + count + 1);
            }
            return count;
        }

        // Версия и имя кодировки проверяются теми же правилами, что и в XML-декларации
        // The version and the encoding name are checked by the same rules as in the XML declaration
        void validateTextDeclaration(XmlScanner& scanner, const std::string& version,
            const std::string& encoding)
        {
            XmlDeclaration declaration;

            declaration.version = version;
            declaration.encoding = encoding;
            try
            {
                validateXmlDeclaration(declaration);
            }
            catch (const DomException& problem)
            {
                scanner.fail(problem.getMessage());
            }
            if (version != "1.0")
                scanner.fail(i18n::format(i18n::Message::ENTITY_XML_VERSION, {version}));
        }
    }

    // Пробелы до имени считаются, но поглощаются только вместе с найденным псевдо-атрибутом
    // The spaces before the name are counted but consumed only together with the found pseudo-attribute
    bool readPseudoAttribute(XmlScanner& scanner, std::string_view name, std::string& value)
    {
        std::size_t spaces = countWhitespaceAhead(scanner);
        std::size_t nameStart = scanner.getOffset() + spaces;

        if (scanner.slice(nameStart, nameStart + name.size()) != name)
            return false;
        if (spaces == 0)
            scanner.fail(i18n::format(i18n::Message::SPACE_BEFORE, {name}));
        scanner.advance(spaces + name.size());
        scanner.skipWhitespace();
        scanner.expect("=");
        scanner.skipWhitespace();
        char quote = scanner.peek();
        if (quote != '"' && quote != '\'')
            scanner.fail(i18n::format(i18n::Message::PSEUDO_ATTRIBUTE_QUOTES));
        scanner.advance(1);
        value = std::string(scanner.readUntil(std::string_view(&quote, 1)));
        return true;
    }

    // "<?xml" + пробел в самом начале — это текстовая декларация, и она обязана быть корректной
    // "<?xml" + a space at the very start is a text declaration, and it must be valid
    std::string_view stripTextDeclaration(std::string_view text)
    {
        if (text.size() < 6 || text.substr(0, 5) != "<?xml" || !utils::isWhitespace(text[5]))
            return text;
        XmlScanner scanner(text);
        std::string version = "1.0";
        std::string encoding;

        scanner.advance(5);
        readPseudoAttribute(scanner, "version", version);
        if (!readPseudoAttribute(scanner, "encoding", encoding))
            scanner.fail(i18n::format(i18n::Message::TEXT_DECLARATION_ENCODING));
        scanner.skipWhitespace();
        if (!scanner.startsWith("?>"))
            scanner.fail(i18n::format(i18n::Message::TEXT_DECLARATION_ORDER));
        scanner.advance(2);
        validateTextDeclaration(scanner, version, encoding);
        return text.substr(scanner.getOffset());
    }
}
