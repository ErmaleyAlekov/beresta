#include "xsd/internal/stream_identity.hpp"

namespace beresta::xsd::detail
{
    // Разбор выражений ограничения — один раз за проход
    // Parsing the expressions of a constraint — once per pass
    const CompiledConstraint& StreamIdentity::compile(const IdentityConstraint& constraint)
    {
        auto found = _compiled.find(&constraint);

        if (found != _compiled.end())
            return found->second;
        CompiledConstraint compiled;
        compiled.streamable = parseStreamPaths(constraint.selector.getSource(), constraint.context, compiled.selector);
        for (const xpath::XPathExpression& field : constraint.fields)
        {
            compiled.fields.emplace_back();
            if (!parseStreamPaths(field.getSource(), constraint.context, compiled.fields.back()))
                compiled.streamable = false;
        }
        return _compiled.emplace(&constraint, std::move(compiled)).first->second;
    }

    /*
    ** Селекторы открытых областей: элемент становится целевым, если до
    ** него доходит хотя бы один вариант селектора (варианты, выбирающие
    ** атрибуты, целей не дают — как и у проверки по дереву). Шаг пути цели
    ** закрепляется сразу: сообщения о повторах придут после её закрытия.
    **
    ** The selectors of the open scopes: the element becomes a target if
    ** at least one selector alternative reaches it (alternatives selecting
    ** attributes give no targets — as in the check over a tree). The path step of the target
    ** is pinned at once: messages about repetitions will come after it is closed.
    */
    void StreamIdentity::selectTargets(const IdentityStart& start)
    {
        for (std::size_t scope = 0; scope < _scopes.size(); ++scope)
        {
            for (std::size_t entry = 0; entry < _scopes[scope].constraints.size(); ++entry)
            {
                const ScopeConstraint& constraint = _scopes[scope].constraints[entry];
                bool selected = false;
                for (const StreamPath& path : constraint.compiled->selector)
                    selected = selected || (!path.attribute && pathReaches(path, _names, _scopes[scope].level, start.level));
                if (!selected)
                    continue;
                if (_targetCount == _targets.size())
                    _targets.emplace_back();
                ActiveTarget& target = _targets[_targetCount++];
                target = ActiveTarget{scope, entry, start.level, start.ordinal, _paths.pin(start.level),
                    start.position, std::vector<FieldState>(constraint.compiled->fields.size())};
            }
        }
    }

    /*
    ** Поля цели: элемент выбран полем, если до него доходит элементный
    ** вариант; узлы считаются без повторов (один элемент — один узел, как
    ** в множестве узлов XPath). Значение элемента станет известно при его
    ** закрытии, поэтому его текст нужно копить.
    **
    ** The target fields: an element is selected by a field if an element
    ** alternative reaches it; nodes are counted without repetitions (one element — one node, as
    ** in an XPath node set). The element value becomes known when it is
    ** closed, so its text has to be accumulated.
    */
    bool StreamIdentity::matchFields(ActiveTarget& target, const IdentityStart& start)
    {
        const CompiledConstraint& compiled = *_scopes[target.scope].constraints[target.constraint].compiled;
        bool needText = false;

        for (std::size_t index = 0; index < target.fields.size(); ++index)
        {
            FieldState& field = target.fields[index];
            bool reached = false;
            for (const StreamPath& path : compiled.fields[index])
                reached = reached || (!path.attribute && pathReaches(path, _names, target.level, start.level));
            if (reached && ++field.matches == 1)
            {
                field.fromElement = true;
                field.elementOrdinal = start.ordinal;
                needText = true;
            }
            matchAttributes(field, compiled.fields[index], target, start);
            matchDefaults(field, compiled.fields[index], target, start);
        }
        return needText;
    }
}
