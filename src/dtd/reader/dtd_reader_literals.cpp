#include "dtd/dtd_reader.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::dtd
{
    // Nmtoken — непустая последовательность символов имени (может начинаться с цифры)
    std::string DtdReader::readNmtoken()
    {
        popFinished();
        detail::XmlScanner& scanner = current();
        std::size_t start = scanner.getOffset();

        while (!scanner.isAtEnd() && utils::isNameChar(scanner.peek()))
            scanner.advance(1);
        if (scanner.getOffset() == start)
            fail("ожидался NMTOKEN");
        return std::string(scanner.slice(start, scanner.getOffset()));
    }

    // Литерал: открывающая кавычка определяет закрывающую
    std::string DtdReader::readQuoted()
    {
        popFinished();
        detail::XmlScanner& scanner = current();
        char quote = scanner.peek();

        if (quote != '"' && quote != '\'')
            fail("ожидался литерал в кавычках");
        scanner.advance(1);
        return std::string(scanner.readUntil(std::string_view(&quote, 1)));
    }

    /*
    ** SYSTEM "sys" | PUBLIC "pub" "sys". Для NOTATION допустимо PUBLIC "pub"
    ** без системной части. Если ни одного ключевого слова нет — пустой результат.
    */
    ExternalId DtdReader::readExternalId(bool publicOnlyAllowed)
    {
        ExternalId id;

        if (startsWith("SYSTEM"))
        {
            advance(6);
            requireSpace(_context.externalSubset);
            id.systemId = readQuoted();
            return id;
        }
        if (!startsWith("PUBLIC"))
            return id;
        advance(6);
        requireSpace(_context.externalSubset);
        id.publicId = readQuoted();
        std::size_t spaces = skipSpaces(_context.externalSubset);
        if (peek() != '"' && peek() != '\'')
        {
            if (!publicOnlyAllowed)
                fail("после PUBLIC-идентификатора ожидался системный идентификатор");
            return id;
        }
        if (spaces == 0)
            fail("ожидался пробел");
        id.systemId = readQuoted();
        return id;
    }

    // Ошибка в позиции текущего сканера
    void DtdReader::fail(const std::string& message)
    {
        current().fail(message);
    }

    // Общий контекст разбора
    DtdContext& DtdReader::getContext() noexcept
    {
        return _context;
    }
}
