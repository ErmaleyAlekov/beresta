#ifndef DTD_READER_HPP
#define DTD_READER_HPP

#include "dtd/dtd_model.hpp"
#include "parser/internal/expansion_budget.hpp"
#include "parser/parser_options.hpp"
#include "parser/xml_scanner.hpp"

#include <deque>
#include <string>
#include <string_view>

namespace beresta::dtd
{
    /*
    ** Общий контекст разбора DTD: модель, настройки, бюджет раскрытия
    ** и признак внешнего подмножества (в нём разрешено больше: условные
    ** секции и ссылки %pe; внутри объявлений и значений сущностей).
    */
    struct DtdContext
    {
        DtdModel& model;
        const ParserOptions& options;
        detail::ExpansionBudget& budget;
        bool externalSubset;
    };

    // Внешний идентификатор: SYSTEM "sys" или PUBLIC "pub" "sys"
    struct ExternalId
    {
        std::string publicId;
        std::string systemId;
    };

    /*
    ** DtdReader — лексический уровень DTD поверх стека сканеров.
    ** Ссылка %name; между токенами раскрывается прозрачно: текст замены
    ** кладётся на вершину стека, а исчерпанный текст снимается. Граница
    ** сущности считается пробелом — так требует стандарт (текст замены
    ** «окружается» пробелами). Стек — std::deque: ссылки на элементы
    ** не инвалидируются при добавлении новых.
    */
    class DtdReader
    {
    public:
        DtdReader(detail::XmlScanner& base, DtdContext& context);

        // Текущий сканер (вершина стека)
        detail::XmlScanner& current();

        // Исчерпан ли весь ввод (включая все раскрытые сущности)
        bool atEnd();

        // Пропускает пробелы; при allowReferences раскрывает %pe; ; возвращает число «пробелов»
        std::size_t skipSpaces(bool allowReferences);

        // Требует хотя бы один пробел
        void requireSpace(bool allowReferences);

        char peek();
        bool startsWith(std::string_view fragment);
        void advance(std::size_t count);
        void expect(std::string_view fragment);

        // Имя и Nmtoken (Nmtoken может начинаться с цифры, '-' или '.')
        std::string readName();
        std::string readNmtoken();

        // Литерал в кавычках "..." или '...' (целиком внутри одного сканера)
        std::string readQuoted();

        // Внешний идентификатор; publicOnlyAllowed — для NOTATION (системная часть необязательна)
        ExternalId readExternalId(bool publicOnlyAllowed);

        [[noreturn]] void fail(const std::string& message);

        DtdContext& getContext() noexcept;

    private:
        // Снимает исчерпанные сущности с вершины стека
        void popFinished();

        // Раскрывает %name; в текущей позиции
        void expandReference();

        detail::XmlScanner& _base;
        DtdContext& _context;
        std::deque<detail::XmlScanner> _frames;
        std::deque<std::string> _frameNames;
    };

    // Текст параметрической сущности (загружает внешнюю при первом обращении)
    std::string_view parameterEntityText(DtdContext& context, std::string_view name);

    // Загружает текст внешней сущности через options.resolver (ошибка — RESOURCE_ERROR)
    void loadExternalEntityText(const ParserOptions& options, EntityDeclaration& entity);
}

#endif
