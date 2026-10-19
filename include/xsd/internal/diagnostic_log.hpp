#ifndef XSD_DIAGNOSTIC_LOG_HPP
#define XSD_DIAGNOSTIC_LOG_HPP

#include "core/source_location.hpp"
#include "parser/error_handler.hpp"
#include "sax/locator.hpp"
#include "xsd/internal/path_tracker.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace beresta::xsd::detail
{
    /*
    ** Группы сообщений в итоговом списке — в том порядке, в каком их давал
    ** обход готового дерева: сначала проверка элементов, затем висячие
    ** IDREF, затем таблицы xs:key / xs:unique и, наконец, xs:keyref.
    **
    ** The message groups in the final list — in the order the traversal of a ready
    ** tree gave them: first the element checks, then dangling
    ** IDREFs, then the xs:key / xs:unique tables and, finally, xs:keyref.
    */
    enum class OrderGroup : std::size_t
    {
        ELEMENTS = 0,
        REFERENCES = 1,
        KEYS = 2,
        KEYREFS = 3
    };

    /*
    ** Этап проверки элемента. Обход дерева проверял элемент целиком
    ** (xsi:type, abstract, nil, атрибуты, текст, модель, xs:any детей) до
    ** его детей; в потоке часть итогов известна лишь при закрытии. Ключ
    ** «элемент + этап» восстанавливает прежний порядок сообщений.
    **
    ** The element check phase. The tree traversal checked an element entirely
    ** (xsi:type, abstract, nil, attributes, text, model, xs:any of the children) before
    ** its children; in a stream part of the results is known only on closing. The key
    ** "element + phase" restores the former order of messages.
    */
    enum class CheckPhase : std::size_t
    {
        XSI_TYPE = 1,
        ABSTRACT = 2,
        NIL = 3,
        SIMPLE_CHILDREN = 4,
        ATTRIBUTES = 5,
        VALUE = 6,
        MODEL = 7,
        WILDCARD = 8,
        ASSERTION = 9
    };

    /*
    ** Ключ порядка сообщения: группа, владелец (номер элемента в прямом
    ** порядке документа или области ключей), этап, подэтап и номер
    ** объекта внутри (например, целевого элемента ключа). Сообщения с
    ** равными ключами остаются в порядке поступления.
    **
    ** The order key of a message: the group, the owner (the number of the element in
    ** document order or of the key scope), the phase, the subphase and the number
    ** of the object inside (for example, the target element of a key). Messages with
    ** equal keys stay in the order of arrival.
    */
    struct DiagnosticOrder
    {
        std::size_t group;
        std::size_t owner;
        std::size_t phase;
        std::size_t sub;
        std::size_t item;
    };

    /*
    ** Позиция события: дешёвая метка парсера (строка и столбец будут
    ** вычислены в конце документа и только для ошибок) или уже готовая
    ** позиция, если источник событий меток не умеет.
    **
    ** The position of an event: a cheap parser mark (the line and column will be
    ** computed at the end of the document and only for errors) or an already ready
    ** position if the event source cannot do marks.
    */
    struct EventPosition
    {
        std::size_t mark = Locator::NO_MARK;
        SourceLocation location;
    };

    // Метки — в позиции, по возрастанию меток (счёт строк одним проходом)
    // Marks — into positions, in ascending order of marks (line counting in one pass)
    void resolvePositions(std::vector<EventPosition*>& positions, const Locator& locator);

    // Сообщение до сборки текста пути: шаг пути или готовый путь (если шага нет)
    // A message before the path text is assembled: a path step or a ready path (if there is no step)
    struct PendingDiagnostic
    {
        DiagnosticOrder order;
        std::size_t step;
        std::string path;
        std::string message;
        EventPosition position;
    };

    /*
    ** DiagnosticLog — накопитель ошибок проверки. Текст «путь: сообщение»
    ** собирается в render(), когда документ дочитан: только тогда известны
    ** номера одноимённых соседей во всех путях.
    **
    ** DiagnosticLog — the accumulator of validation errors. The text "path: message"
    ** is assembled in render(), when the document has been read to the end: only then are
    ** the numbers of same-named siblings known in all paths.
    */
    class DiagnosticLog
    {
    public:
        DiagnosticLog();

        // Ошибка у закреплённого шага пути
        // An error at a pinned path step
        void add(const DiagnosticOrder& order, std::size_t step, std::string message,
            const EventPosition& position);

        // Ошибка с уже готовым текстом пути (элемент известен лишь по дереву)
        // An error with a ready path text (the element is known only from the tree)
        void addAtPath(const DiagnosticOrder& order, std::string path, std::string message,
            const EventPosition& position);

        // Перевод меток всех сообщений в строки и столбцы (пока парсер жив — до конца разбора)
        // Converting the marks of all messages into lines and columns (while the parser is alive — before the end of parsing)
        void resolve(const Locator& locator);

        // Итоговый список в порядке ключей с собранными путями
        // The final list in key order with assembled paths
        std::vector<Diagnostic> render(const PathTracker& paths);

    private:
        std::vector<PendingDiagnostic> _pending;
    };
}

#endif
