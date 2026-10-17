#include "dtd/dtd_reader.hpp"
#include "utils/xml_chars.hpp"

namespace beresta::dtd
{
    // Базовый сканер принадлежит вызывающему; стек сущностей изначально пуст
    DtdReader::DtdReader(detail::XmlScanner& base, DtdContext& context)
        : _base(base), _context(context), _frames(), _frameNames()
    {
    }

    // Вершина стека или базовый сканер
    detail::XmlScanner& DtdReader::current()
    {
        if (_frames.empty())
            return _base;
        return _frames.back();
    }

    // Конец — когда все раскрытые сущности исчерпаны и базовый ввод закончился
    bool DtdReader::atEnd()
    {
        popFinished();
        return _frames.empty() && _base.isAtEnd();
    }

    /*
    ** Пробелы, концы раскрытых сущностей и (если разрешено) сами ссылки
    ** %name; — всё это разделители токенов. Ссылка распознаётся только
    ** если за '%' сразу идёт символ имени: в "<!ENTITY % name" это не так.
    */
    std::size_t DtdReader::skipSpaces(bool allowReferences)
    {
        std::size_t count = 0;

        while (true)
        {
            count += current().skipWhitespace();
            detail::XmlScanner& scanner = current();
            if (scanner.isAtEnd() && !_frames.empty())
            {
                popFinished();
                ++count;
                continue;
            }
            if (!allowReferences || scanner.peek() != '%')
                return count;
            std::string_view rest = scanner.slice(scanner.getOffset() + 1, scanner.getOffset() + 2);
            if (rest.empty() || !utils::isNameStartChar(rest[0]))
                return count;
            expandReference();
            ++count;
        }
    }

    // Обязательный разделитель
    void DtdReader::requireSpace(bool allowReferences)
    {
        if (skipSpaces(allowReferences) == 0)
            fail("ожидался пробел");
    }
}
