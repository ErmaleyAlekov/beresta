#include "xsd/internal/tree_identity.hpp"
#include "xsd/internal/identity_common.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** Сначала таблицы xs:key / xs:unique во всех областях (в прямом порядке
    ** областей), затем xs:keyref — ссылка может указывать на ключ из
    ** области-потомка, поэтому все таблицы должны быть готовы заранее.
    **
    ** First the xs:key / xs:unique tables in all scopes (in document order of the
    ** scopes), then xs:keyref — a reference may point to a key from a
    ** descendant scope, so all tables must be ready in advance.
    */
    void TreeIdentityCheck::checkConstraints()
    {
        for (const IdentityScope& scope : _scopes)
        {
            for (const IdentityConstraint* constraint : scope.declaration->identityConstraints)
            {
                if (constraint->kind != IdentityKind::KEYREF)
                    buildKeyTable(scope, *constraint);
            }
        }
        for (const IdentityScope& scope : _scopes)
        {
            for (const IdentityConstraint* constraint : scope.declaration->identityConstraints)
            {
                if (constraint->kind == IdentityKind::KEYREF)
                    checkKeyReferences(scope, *constraint);
            }
        }
    }

    // Таблица ключей области: повтор значения — нарушение уникальности
    // The key table of a scope: a repeated value is a uniqueness violation
    void TreeIdentityCheck::buildKeyTable(const IdentityScope& scope, const IdentityConstraint& constraint)
    {
        KeyTable table{scope.element, &constraint, {}};

        for (KeyRow& row : selectRows(scope, constraint))
        {
            if (!table.keys.insert(row.canonical).second)
                reportElement(*row.target, i18n::format(i18n::Message::KEY_DUPLICATE,
                    {joinKeyValues(row.shown), uniquenessLabel(constraint), constraint.name}));
        }
        _tables.push_back(std::move(table));
    }

    /*
    ** Проверка xs:keyref в области scope: значение каждой ссылки (если все
    ** её поля есть) обязано найтись среди значений ключа refer, объявленного
    ** на самом элементе-области или на его потомках.
    **
    ** Checks xs:keyref in the scope scope: the value of each reference (if all
    ** its fields are present) must be found among the values of the refer key declared
    ** on the scope element itself or on its descendants.
    */
    void TreeIdentityCheck::checkKeyReferences(const IdentityScope& scope, const IdentityConstraint& keyref)
    {
        if (keyref.referenced == nullptr)
            return;
        for (const KeyRow& row : selectRows(scope, keyref))
        {
            if (!keyExists(*scope.element, *keyref.referenced, row.canonical))
                reportElement(*row.target, i18n::format(i18n::Message::KEYREF_NOT_FOUND,
                    {keyref.name, joinKeyValues(row.shown), keyref.referenced->name}));
        }
    }

    // Есть ли значение в таблицах ключа key, построенных в scope или его потомках
    // Whether the value is in the tables of the key key built in scope or its descendants
    bool TreeIdentityCheck::keyExists(const Element& scope, const IdentityConstraint& key,
        const std::vector<std::string>& values) const
    {
        for (const KeyTable& table : _tables)
        {
            if (table.constraint != &key)
                continue;
            bool inside = table.scope == &scope || table.scope->isDescendantOf(scope);
            if (inside && table.keys.count(values) != 0)
                return true;
        }
        return false;
    }
}
