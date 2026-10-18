#include "xsd/internal/validation_run.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Ограничения идентичности проверяются после основного прохода: к этому
    ** моменту известны типы всех элементов и атрибутов (значения ключей
    ** сравниваются по значению типа, а не по строке). Сначала строятся
    ** таблицы xs:key / xs:unique во всех областях, затем проверяются
    ** xs:keyref — ссылка может указывать на ключ из области-потомка.
    **
    ** Identity constraints are checked after the main pass: by that
    ** moment the types of all elements and attributes are known (key values
    ** are compared by the type value, not by the string). First the
    ** xs:key / xs:unique tables are built in all scopes, then the
    ** xs:keyref are checked — a reference may point to a key from a descendant scope.
    */
    void ValidationRun::checkIdentityConstraints()
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

    /*
    ** Целевые элементы области и значения их полей. Строка попадает в
    ** результат, только если есть все поля; у xs:key отсутствие поля —
    ** ошибка (ключ обязателен), у xs:unique и xs:keyref — просто пропуск.
    **
    ** The target elements of a scope and their field values. A row gets into
    ** the result only if all fields are present; for xs:key a missing field is
    ** an error (the key is mandatory), for xs:unique and xs:keyref it is just skipped.
    */
    std::vector<KeyRow> ValidationRun::selectRows(const IdentityScope& scope, const IdentityConstraint& constraint)
    {
        std::vector<KeyRow> rows;
        xpath::XPathValue selected = constraint.selector.evaluate(*scope.element, constraint.context);

        if (selected.getType() != xpath::ValueType::NODE_SET)
        {
            report(*scope.element, scope.path, i18n::format(i18n::Message::IDENTITY_SELECTOR_NOT_NODESET, {constraint.name}));
            return rows;
        }
        for (const xpath::XPathItem& item : selected.getNodes())
        {
            if (item.attribute != nullptr || item.node->getType() != NodeType::ELEMENT)
                continue;
            KeyRow row{static_cast<const Element*>(item.node), {}, {}};
            bool complete = true;
            for (const xpath::XPathExpression& field : constraint.fields)
                complete = fieldValue(*row.target, field, constraint, row) && complete;
            if (complete)
                rows.push_back(std::move(row));
            else if (constraint.kind == IdentityKind::KEY)
                reportElement(*row.target, i18n::format(i18n::Message::KEY_FIELD_MISSING, {constraint.name}));
        }
        return rows;
    }

    /*
    ** Значение одного поля. Поле обязано выбрать не больше одного узла, и
    ** этот узел — атрибут или элемент простого типа. Значение приводится к
    ** каноническому виду своего типа: "1.0" и "1" для decimal — один ключ.
    **
    ** The value of one field. A field must select at most one node, and
    ** this node is an attribute or an element of a simple type. The value is converted to
    ** the canonical form of its type: "1.0" and "1" for decimal are one key.
    */
    bool ValidationRun::fieldValue(const Element& target, const xpath::XPathExpression& field,
        const IdentityConstraint& constraint, KeyRow& row)
    {
        xpath::XPathValue value = field.evaluate(target, constraint.context);
        if (value.getType() != xpath::ValueType::NODE_SET || value.getNodes().size() > 1)
        {
            reportElement(target, i18n::format(i18n::Message::KEY_FIELD_SEVERAL_NODES, {constraint.name}));
            return false;
        }
        if (value.getNodes().empty())
            return false;
        const xpath::XPathItem& item = value.getNodes().front();
        if (item.attribute == nullptr && static_cast<const Element*>(item.node)->findFirstOfType(NodeType::ELEMENT)
            != ContainerNode::npos)
        {
            reportElement(target, i18n::format(i18n::Message::KEY_FIELD_COMPLEX, {constraint.name}));
            return false;
        }
        std::string text = xpath::stringValue(item);
        const SimpleType* type = typeOfItem(item);
        if (type == nullptr)
            type = builtinType("string");
        row.canonical.push_back(canonicalValue(*type, text));
        row.shown.push_back(normalizeWhiteSpace(text, WhiteSpace::COLLAPSE));
        return true;
    }

    // Таблица ключей области: повтор значения — нарушение уникальности
    // The key table of a scope: a repeated value is a uniqueness violation
    void ValidationRun::buildKeyTable(const IdentityScope& scope, const IdentityConstraint& constraint)
    {
        KeyTable table{scope.element, &constraint, {}};
        std::string label = "xs:unique";

        if (constraint.kind == IdentityKind::KEY)
            label = "xs:key";
        for (KeyRow& row : selectRows(scope, constraint))
        {
            if (!table.keys.insert(row.canonical).second)
                reportElement(*row.target, i18n::format(i18n::Message::KEY_DUPLICATE, {joinKeyValues(row.shown), label, constraint.name}));
        }
        _tables.push_back(std::move(table));
    }

    // Значения полей для сообщения: "A1, 2026"
    // The field values for a message: "A1, 2026"
    std::string joinKeyValues(const std::vector<std::string>& values)
    {
        std::string result;

        for (const std::string& value : values)
        {
            if (!result.empty())
                result += ", ";
            result += value;
        }
        return result;
    }
}
