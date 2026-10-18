#include "xsd/internal/stream_identity.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    // Стратегия пишет в общие пути и журнал валидатора
    // The strategy writes into the common paths and log of the validator
    StreamIdentity::StreamIdentity(PathTracker& paths, DiagnosticLog& log)
        : _paths(paths), _log(log), _compiled(), _reportedUnstreamable(), _names(), _scopes(), _targets(),
          _targetCount(0), _tables(), _lastOrdinal(0)
    {
    }

    /*
    ** Новая область: её ограничения, разобранные для потока. Ограничение с
    ** выражением вне грамматики XSD потоково не проверить — об этом одно
    ** сообщение на ограничение за документ, чтобы не завалить список ошибок.
    **
    ** A new scope: its constraints parsed for the stream. A constraint with
    ** an expression outside the XSD grammar cannot be checked in a stream — there is one
    ** message about it per constraint per document, so as not to flood the error list.
    */
    void StreamIdentity::openScope(std::size_t level, std::size_t ordinal, const EventPosition& position,
        const ElementDecl& declaration)
    {
        _scopes.push_back(StreamScope{level, ordinal, {}});
        for (std::size_t index = 0; index < declaration.identityConstraints.size(); ++index)
        {
            const IdentityConstraint& constraint = *declaration.identityConstraints[index];
            const CompiledConstraint& compiled = compile(constraint);
            if (compiled.streamable)
            {
                _scopes.back().constraints.push_back(ScopeConstraint{&constraint, &compiled, index, {}});
                continue;
            }
            if (!_reportedUnstreamable.insert(&constraint).second)
                continue;
            std::size_t group = static_cast<std::size_t>(OrderGroup::KEYS);
            if (constraint.kind == IdentityKind::KEYREF)
                group = static_cast<std::size_t>(OrderGroup::KEYREFS);
            _log.add(DiagnosticOrder{group, ordinal, index, 0, 0}, _paths.pin(level),
                i18n::format(i18n::Message::IDENTITY_NOT_STREAMABLE, {constraint.name}), position);
        }
    }

    /*
    ** Начало элемента: его имя ложится в стек имён, затем селекторы
    ** открытых областей могут сделать его целевым, а поля активных целей —
    ** выбрать его самого или его атрибуты. true — нужен текст элемента.
    **
    ** An element start: its name goes into the name stack, then the selectors
    ** of the open scopes may make it a target, and the fields of the active targets may
    ** select it itself or its attributes. true — the element text is needed.
    */
    bool StreamIdentity::elementStarted(const IdentityStart& start)
    {
        bool needText = false;

        _lastOrdinal = start.ordinal;
        if (_names.size() <= start.level)
            _names.resize(start.level + 1);
        _names[start.level].namespaceUri.assign(start.element.namespaceUri.data(), start.element.namespaceUri.size());
        _names[start.level].localName.assign(start.element.localName.data(), start.element.localName.size());
        selectTargets(start);
        for (std::size_t index = 0; index < _targetCount; ++index)
            needText = matchFields(_targets[index], start) || needText;
        return needText;
    }

    /*
    ** Конец элемента: значения полей, выбравших его, затем строки целей,
    ** закрывающихся здесь (в порядке их появления), и, наконец, области.
    ** Цели и области вложены как элементы, поэтому закрывающиеся всегда
    ** лежат в конце своих списков.
    **
    ** An element end: the values of the fields that selected it, then the rows of the targets
    ** closing here (in the order of their appearance), and, finally, the scopes.
    ** Targets and scopes are nested like elements, so the closing ones always
    ** lie at the end of their lists.
    */
    void StreamIdentity::elementEnded(const IdentityEnd& end)
    {
        std::size_t first = _targetCount;

        captureElement(end);
        while (first > 0 && _targets[first - 1].level == end.level)
            --first;
        for (std::size_t index = first; index < _targetCount; ++index)
            finishTarget(_targets[index]);
        _targetCount = first;
        while (!_scopes.empty() && _scopes.back().level == end.level)
        {
            closeScope(_scopes.back());
            _scopes.pop_back();
        }
    }

    // Все области закрылись вместе со своими элементами, а метки их сообщений переведёт журнал
    // All scopes were closed together with their elements, and the log converts the marks of their messages
    void StreamIdentity::finishDocument(const Locator* locator)
    {
        (void)locator;
    }
}
