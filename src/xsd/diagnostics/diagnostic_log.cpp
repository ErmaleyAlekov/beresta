#include "xsd/internal/diagnostic_log.hpp"

#include <algorithm>
#include <utility>

namespace beresta::xsd::detail
{
    // Журнал пуст до первой ошибки
    // The log is empty until the first error
    DiagnosticLog::DiagnosticLog()
        : _pending()
    {
    }

    // Путь будет собран по шагу в render()
    // The path will be assembled from the step in render()
    void DiagnosticLog::add(const DiagnosticOrder& order, std::size_t step, std::string message,
        const EventPosition& position)
    {
        _pending.push_back(PendingDiagnostic{order, step, std::string(), std::move(message), position});
    }

    // Путь уже готов — шаг не нужен
    // The path is already ready — no step is needed
    void DiagnosticLog::addAtPath(const DiagnosticOrder& order, std::string path, std::string message,
        const EventPosition& position)
    {
        _pending.push_back(PendingDiagnostic{order, NO_STEP, std::move(path), std::move(message), position});
    }

    // Лексикографическое сравнение ключей порядка (группа, владелец, этап, подэтап, объект)
    // A lexicographic comparison of order keys (group, owner, phase, subphase, object)
    static bool comesBefore(const PendingDiagnostic& left, const PendingDiagnostic& right) noexcept
    {
        const DiagnosticOrder& a = left.order;
        const DiagnosticOrder& b = right.order;

        if (a.group != b.group)
            return a.group < b.group;
        if (a.owner != b.owner)
            return a.owner < b.owner;
        if (a.phase != b.phase)
            return a.phase < b.phase;
        if (a.sub != b.sub)
            return a.sub < b.sub;
        return a.item < b.item;
    }

    /*
    ** Устойчивая сортировка сохраняет порядок поступления равных ключей;
    ** обычно сообщения уже упорядочены, и сортировка пропускается. Затем
    ** каждое сообщение получает путь и позицию элемента в исходнике.
    **
    ** A stable sort keeps the order of arrival of equal keys;
    ** usually the messages are already ordered, and the sort is skipped. Then
    ** every message gets the path and the element position in the source.
    */
    std::vector<Diagnostic> DiagnosticLog::render(const PathTracker& paths)
    {
        std::vector<Diagnostic> result;

        if (!std::is_sorted(_pending.begin(), _pending.end(), comesBefore))
            std::stable_sort(_pending.begin(), _pending.end(), comesBefore);
        result.reserve(_pending.size());
        for (PendingDiagnostic& pending : _pending)
        {
            std::string path = std::move(pending.path);
            if (pending.step != NO_STEP)
                path = paths.text(pending.step);
            result.push_back(Diagnostic{ErrorSeverity::RECOVERABLE_ERROR, path + ": " + pending.message,
                ParsePosition{0, pending.position.location.line, pending.position.location.column}});
        }
        _pending.clear();
        return result;
    }
}
