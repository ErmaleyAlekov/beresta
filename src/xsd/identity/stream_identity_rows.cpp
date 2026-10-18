#include "xsd/internal/stream_identity.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** Закрылся элемент, выбранный полем: его значение — накопленный текст
    ** и тип корректного простого значения. Элемент с детьми-элементами
    ** простым не является — такое поле ошибочно.
    **
    ** An element selected by a field has closed: its value is the accumulated text
    ** and the type of a valid simple value. An element with child elements
    ** is not simple — such a field is erroneous.
    */
    void StreamIdentity::captureElement(const IdentityEnd& end)
    {
        for (std::size_t index = 0; index < _targetCount; ++index)
        {
            for (FieldState& field : _targets[index].fields)
            {
                if (!field.fromElement || field.elementOrdinal != end.ordinal)
                    continue;
                field.complex = end.hasChildElement;
                field.value.assign(end.text.data(), end.text.size());
                field.type = end.valueType;
            }
        }
    }

    /*
    ** Строка цели: попадает в область, только если есть все поля; у
    ** xs:key отсутствие поля — ошибка (ключ обязателен), у xs:unique и
    ** xs:keyref — просто пропуск. Все поля проверяются, даже после ошибки.
    **
    ** The target row: it gets into the scope only if all fields are present; for
    ** xs:key a missing field is an error (the key is mandatory), for xs:unique and
    ** xs:keyref it is just skipped. All fields are checked, even after an error.
    */
    void StreamIdentity::finishTarget(const ActiveTarget& target)
    {
        ScopeConstraint& entry = _scopes[target.scope].constraints[target.constraint];
        StreamRow row{target.ordinal, target.step, target.position, {}, {}};
        bool complete = true;

        for (std::size_t index = 0; index < target.fields.size(); ++index)
            complete = fieldRowValue(target, index, row) && complete;
        if (complete)
            entry.rows.push_back(std::move(row));
        else if (entry.constraint->kind == IdentityKind::KEY)
            reportTarget(target, 0, i18n::format(i18n::Message::KEY_FIELD_MISSING, {entry.constraint->name}));
    }

    /*
    ** Значение одного поля: не больше одного узла, и это атрибут или
    ** простой элемент. Значение приводится к каноническому виду своего
    ** типа ("1.0" и "1" для decimal — один ключ); тип неизвестен — строка.
    **
    ** The value of one field: at most one node, and it is an attribute or
    ** a simple element. The value is converted to the canonical form of its
    ** type ("1.0" and "1" for decimal are one key); the type is unknown — a string.
    */
    bool StreamIdentity::fieldRowValue(const ActiveTarget& target, std::size_t index, StreamRow& row)
    {
        const FieldState& field = target.fields[index];
        const IdentityConstraint& constraint = *_scopes[target.scope].constraints[target.constraint].constraint;

        if (field.matches > 1)
        {
            reportTarget(target, 0, i18n::format(i18n::Message::KEY_FIELD_SEVERAL_NODES, {constraint.name}));
            return false;
        }
        if (field.matches == 0)
            return false;
        if (field.fromElement && field.complex)
        {
            reportTarget(target, 0, i18n::format(i18n::Message::KEY_FIELD_COMPLEX, {constraint.name}));
            return false;
        }
        const SimpleType* type = field.type;
        if (type == nullptr)
            type = builtinType("string");
        row.canonical.push_back(canonicalValue(*type, field.value));
        row.shown.push_back(normalizeWhiteSpace(field.value, WhiteSpace::COLLAPSE));
        return true;
    }

    // Ошибка у цели: место в порядке — область, ограничение, подэтап и номер цели
    // An error at a target: the place in the order is the scope, the constraint, the subphase and the target number
    void StreamIdentity::reportTarget(const ActiveTarget& target, std::size_t sub, std::string message)
    {
        const StreamScope& scope = _scopes[target.scope];
        const ScopeConstraint& entry = scope.constraints[target.constraint];
        std::size_t group = static_cast<std::size_t>(OrderGroup::KEYS);

        if (entry.constraint->kind == IdentityKind::KEYREF)
            group = static_cast<std::size_t>(OrderGroup::KEYREFS);
        _log.add(DiagnosticOrder{group, scope.ordinal, entry.index, sub, target.ordinal}, target.step,
            std::move(message), target.position);
    }

    // Потоковой стратегии дерево не нужно: всё проверено по ходу событий
    // The streaming strategy does not need a tree: everything was checked along the events
    void StreamIdentity::checkTree(const Document& document)
    {
        (void)document;
    }
}
