#include "xsd/internal/stream_identity.hpp"
#include "xsd/internal/identity_common.hpp"
#include "i18n/i18n.hpp"

#include <algorithm>
#include <utility>

namespace beresta::xsd::detail
{
    // Строки упорядочиваются по номеру цели: вложенные цели закрываются раньше внешних
    // Rows are ordered by the target number: nested targets are closed before the outer ones
    static bool rowBefore(const StreamRow& left, const StreamRow& right) noexcept
    {
        return left.ordinal < right.ordinal;
    }

    /*
    ** Закрытие области: сначала таблицы её xs:key / xs:unique, затем её
    ** xs:keyref. Таблицы областей-потомков к этому моменту уже готовы
    ** (потомки закрылись раньше), поэтому ссылка видит и их ключи.
    **
    ** Closing a scope: first the tables of its xs:key / xs:unique, then its
    ** xs:keyref. The tables of the descendant scopes are already ready by this moment
    ** (the descendants closed earlier), so a reference sees their keys too.
    */
    void StreamIdentity::closeScope(StreamScope& scope)
    {
        for (ScopeConstraint& entry : scope.constraints)
        {
            if (entry.constraint->kind != IdentityKind::KEYREF)
                buildTable(scope, entry);
        }
        for (ScopeConstraint& entry : scope.constraints)
        {
            if (entry.constraint->kind == IdentityKind::KEYREF)
                resolveReferences(scope, entry);
        }
    }

    /*
    ** Таблица ключей области: строки — в порядке документа (повтором
    ** считается более поздняя цель, как у проверки по дереву). Таблица
    ** остаётся до конца документа: её могут искать ссылки областей-предков.
    **
    ** The key table of a scope: the rows are in document order (the later target
    ** is considered the repetition, as in the check over a tree). The table
    ** stays until the end of the document: references of ancestor scopes may look for it.
    */
    void StreamIdentity::buildTable(const StreamScope& scope, ScopeConstraint& entry)
    {
        StreamTable table{entry.constraint, scope.ordinal, _lastOrdinal, {}};

        std::stable_sort(entry.rows.begin(), entry.rows.end(), rowBefore);
        for (const StreamRow& row : entry.rows)
        {
            if (table.keys.insert(row.canonical).second)
                continue;
            _log.add(DiagnosticOrder{static_cast<std::size_t>(OrderGroup::KEYS), scope.ordinal, entry.index, 1,
                row.ordinal}, row.step, i18n::format(i18n::Message::KEY_DUPLICATE,
                {joinKeyValues(row.shown), uniquenessLabel(*entry.constraint), entry.constraint->name}), row.position);
        }
        entry.rows.clear();
        _tables.push_back(std::move(table));
    }

    // Каждая полная строка xs:keyref обязана найтись среди ключей refer в области или её потомках
    // Every complete xs:keyref row must be found among the refer keys in the scope or its descendants
    void StreamIdentity::resolveReferences(const StreamScope& scope, ScopeConstraint& entry)
    {
        const IdentityConstraint& keyref = *entry.constraint;

        std::stable_sort(entry.rows.begin(), entry.rows.end(), rowBefore);
        for (const StreamRow& row : entry.rows)
        {
            if (keyref.referenced == nullptr || keyExists(scope, *keyref.referenced, row.canonical))
                continue;
            _log.add(DiagnosticOrder{static_cast<std::size_t>(OrderGroup::KEYREFS), scope.ordinal, entry.index, 1,
                row.ordinal}, row.step, i18n::format(i18n::Message::KEYREF_NOT_FOUND,
                {keyref.name, joinKeyValues(row.shown), keyref.referenced->name}), row.position);
        }
        entry.rows.clear();
    }

    /*
    ** Таблица принадлежит области или её потомку, если элемент её области
    ** открылся внутри этой области: номера элементов поддерева идут подряд
    ** от номера области до последнего начатого элемента.
    **
    ** A table belongs to the scope or its descendant if the element of its scope
    ** opened inside this scope: the numbers of the subtree elements go in a row
    ** from the scope number up to the last started element.
    */
    bool StreamIdentity::keyExists(const StreamScope& scope, const IdentityConstraint& key,
        const std::vector<std::string>& values) const
    {
        for (const StreamTable& table : _tables)
        {
            bool inside = table.first >= scope.ordinal && table.first <= _lastOrdinal;
            if (table.constraint == &key && inside && table.keys.count(values) != 0)
                return true;
        }
        return false;
    }
}
