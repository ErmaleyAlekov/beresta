#ifndef DTD_READER_HPP
#define DTD_READER_HPP

#include "dtd/dtd_model.hpp"
#include "parser/internal/expansion_budget.hpp"
#include "parser/parser_options.hpp"
#include "parser/xml_scanner.hpp"
#include "sax/content_handler.hpp"

#include <cstddef>
#include <deque>
#include <string>
#include <string_view>

namespace beresta::dtd
{
    /*
    ** Общий контекст разбора DTD: модель, настройки, бюджет раскрытия
    ** и признак внешнего подмножества (в нём разрешено больше: условные
    ** секции и ссылки %pe; внутри объявлений и значений сущностей).
    ** baseUri — база для относительных systemId в объявлениях этого текста:
    ** URI документа для внутреннего подмножества, URI DTD — для внешнего.
    ** handler — получатель PI из DTD (nullptr — PI только проверяются).
    **
    ** The common DTD parsing context: the model, the settings, the expansion budget
    ** and the external subset flag (more is allowed there: conditional
    ** sections and %pe; references inside declarations and entity values).
    ** baseUri is the base for relative systemIds in the declarations of this text:
    ** the document URI for the internal subset, the DTD URI for the external one.
    ** handler is the receiver of PIs from the DTD (nullptr — PIs are only checked).
    */
    struct DtdContext
    {
        DtdModel& model;
        const ParserOptions& options;
        detail::ExpansionBudget& budget;
        bool externalSubset;
        std::string baseUri = std::string();
        ContentHandler* handler = nullptr;
    };

    // Внешний идентификатор: SYSTEM "sys" или PUBLIC "pub" "sys"
    // An external identifier: SYSTEM "sys" or PUBLIC "pub" "sys"
    struct ExternalId
    {
        std::string publicId;
        std::string systemId;
        // Был ли идентификатор вообще: SYSTEM "" — пустой, но присутствующий
        // Whether the identifier was present at all: SYSTEM "" is empty but present
        bool present = false;
    };

    /*
    ** DtdReader — лексический уровень DTD поверх стека сканеров.
    ** Ссылка %name; между токенами раскрывается прозрачно: текст замены
    ** кладётся на вершину стека, а исчерпанный текст снимается. Граница
    ** сущности считается пробелом — так требует стандарт (текст замены
    ** «окружается» пробелами). Стек — std::deque: ссылки на элементы
    ** не инвалидируются при добавлении новых.
    **
    ** Каждый кадр стека получает уникальный номер (у базового текста — 0).
    ** По номерам парсер проверяет, что объявление или группа начались и
    ** закончились в одном и том же тексте (разделы 2.8 и 3.2.1).
    **
    ** DtdReader — the DTD lexical level over a stack of scanners.
    ** A %name; reference between tokens is expanded transparently: the replacement text
    ** is pushed on top of the stack, and an exhausted text is popped. An entity
    ** boundary counts as a space — the standard requires it (the replacement text
    ** is "surrounded" by spaces). The stack is a std::deque: references to elements
    ** are not invalidated when new ones are added.
    ** Each stack frame gets a unique number (the base text has 0).
    ** By the numbers the parser checks that a declaration or a group started and
    ** ended in the same text (sections 2.8 and 3.2.1).
    */
    class DtdReader
    {
    public:
        DtdReader(detail::XmlScanner& base, DtdContext& context);

        // Текущий сканер (вершина стека)
        // The current scanner (the top of the stack)
        detail::XmlScanner& current();

        // Исчерпан ли весь ввод (включая все раскрытые сущности)
        // Whether the whole input is exhausted (including all expanded entities)
        bool atEnd();

        // Пропускает пробелы; при allowReferences раскрывает %pe; ; возвращает число «пробелов»
        // Skips spaces; with allowReferences expands %pe; ; returns the number of "spaces"
        std::size_t skipSpaces(bool allowReferences);

        // Требует хотя бы один пробел
        // Requires at least one space
        void requireSpace(bool allowReferences);

        char peek();
        bool startsWith(std::string_view fragment);
        void advance(std::size_t count);
        void expect(std::string_view fragment);

        // Имя и Nmtoken (Nmtoken может начинаться с цифры, '-' или '.')
        // A Name and an Nmtoken (an Nmtoken may start with a digit, '-' or '.')
        std::string readName();
        std::string readNmtoken();

        // Литерал в кавычках "..." или '...' (целиком внутри одного сканера)
        // A quoted literal "..." or '...' (entirely inside one scanner)
        std::string readQuoted();

        // Внешний идентификатор; publicOnlyAllowed — для NOTATION (системная часть необязательна)
        // An external identifier; publicOnlyAllowed — for NOTATION (the system part is optional)
        ExternalId readExternalId(bool publicOnlyAllowed);

        [[noreturn]] void fail(const std::string& message);
        DtdContext& getContext() noexcept;

        // ---- Кадры параметрических сущностей (dtd_reader_frames.cpp) ----
        // ---- Parameter entity frames (dtd_reader_frames.cpp) ----

        // Номер текста, из которого сейчас читается ввод (0 — базовый текст)
        // The number of the text the input is read from now (0 — the base text)
        std::size_t frameId() const noexcept;

        // Ещё не исчерпан ли текст с этим номером (базовый — всегда)
        // Whether the text with this number is not exhausted yet (the base one — always)
        bool isFrameOpen(std::size_t id) const noexcept;

        // Читаем ли внешнее подмножество или текст внешней PE (на любой глубине)
        // Whether the external subset or the text of an external PE is being read (at any depth)
        bool inExternalText() const noexcept;

        // База для относительных systemId в текущем тексте
        // The base for relative systemIds in the current text
        const std::string& currentBaseUri() const noexcept;

    private:
        // Сведения о раскрытой сущности на стеке
        // Information about an expanded entity on the stack
        struct FrameInfo
        {
            std::string name;
            std::size_t id;
            std::string baseUri;
            bool external;
        };

        // Снимает исчерпанные сущности с вершины стека
        // Pops exhausted entities from the top of the stack
        void popFinished();

        // Раскрывает %name; в текущей позиции
        // Expands %name; at the current position
        void expandReference();

        // PUBLIC "pub" ["sys"] — после ключевого слова PUBLIC
        // PUBLIC "pub" ["sys"] — after the PUBLIC keyword
        void readPublicId(ExternalId& id, bool publicOnlyAllowed);

        detail::XmlScanner& _base;
        DtdContext& _context;
        std::deque<detail::XmlScanner> _frames;
        std::deque<FrameInfo> _frameInfo;
        std::size_t _nextFrameId;
    };

    // Текст параметрической сущности (загружает внешнюю при первом обращении)
    // The text of a parameter entity (loads an external one on first access)
    std::string_view parameterEntityText(DtdContext& context, std::string_view name);

    // Загружает текст внешней сущности через options.resolver (ошибка — RESOURCE_ERROR)
    // Loads the text of an external entity through options.resolver (error — RESOURCE_ERROR)
    void loadExternalEntityText(const ParserOptions& options, EntityDeclaration& entity);
}

#endif
