#include "dtd/dtd_reader.hpp"
#include "i18n/i18n.hpp"
#include "utils/xml_chars.hpp"
#include "utils/xml_tokens.hpp"

namespace beresta::dtd
{
    // Nmtoken — непустая последовательность символов имени (может начинаться с цифры)
    // Nmtoken — a non-empty sequence of name characters (may start with a digit)
    std::string DtdReader::readNmtoken()
    {
        popFinished();
        detail::XmlScanner& scanner = current();
        std::string_view rest = scanner.slice(scanner.getOffset(), static_cast<std::size_t>(-1));
        std::size_t length = 0;
        std::size_t step = utils::nameCharLength(rest, 0, false);

        // Символ за символом, с учётом многобайтовых символов UTF-8
        // Character by character, taking multi-byte UTF-8 characters into account
        while (step > 0)
        {
            length += step;
            step = utils::nameCharLength(rest, length, false);
        }
        if (length == 0)
            fail(i18n::format(i18n::Message::NMTOKEN_EXPECTED));
        scanner.advance(length);
        return std::string(rest.substr(0, length));
    }

    // Литерал: открывающая кавычка определяет закрывающую
    // A literal: the opening quote determines the closing one
    std::string DtdReader::readQuoted()
    {
        popFinished();
        detail::XmlScanner& scanner = current();
        char quote = scanner.peek();

        if (quote != '"' && quote != '\'')
            fail(i18n::format(i18n::Message::QUOTED_LITERAL_EXPECTED));
        scanner.advance(1);
        return std::string(scanner.readUntil(std::string_view(&quote, 1)));
    }

    /*
    ** SYSTEM "sys" | PUBLIC "pub" "sys". Для NOTATION допустимо PUBLIC "pub"
    ** без системной части. Если ни одного ключевого слова нет — пустой
    ** результат с present = false (SYSTEM "" — пустой, но присутствующий).
    **
    ** SYSTEM "sys" | PUBLIC "pub" "sys". For NOTATION PUBLIC "pub" without
    ** the system part is allowed. If there is no keyword at all, the result is empty
    ** with present = false (SYSTEM "" is empty but present).
    */
    ExternalId DtdReader::readExternalId(bool publicOnlyAllowed)
    {
        ExternalId id;

        if (startsWith("SYSTEM"))
        {
            advance(6);
            requireSpace(inExternalText());
            id.systemId = readQuoted();
            id.present = true;
            return id;
        }
        if (!startsWith("PUBLIC"))
            return id;
        advance(6);
        readPublicId(id, publicOnlyAllowed);
        id.present = true;
        return id;
    }

    // Публичный идентификатор допускает только PubidChar (продукция [13])
    // A public identifier allows only PubidChar (production [13])
    void DtdReader::readPublicId(ExternalId& id, bool publicOnlyAllowed)
    {
        requireSpace(inExternalText());
        id.publicId = readQuoted();
        if (!utils::isValidPublicId(id.publicId))
            fail(i18n::format(i18n::Message::PUBLIC_ID_CHARACTER, {id.publicId}));
        std::size_t spaces = skipSpaces(inExternalText());
        if (peek() != '"' && peek() != '\'')
        {
            if (!publicOnlyAllowed)
                fail(i18n::format(i18n::Message::SYSTEM_ID_AFTER_PUBLIC));
            return;
        }
        if (spaces == 0)
            fail(i18n::format(i18n::Message::SPACE_EXPECTED));
        id.systemId = readQuoted();
    }
}
