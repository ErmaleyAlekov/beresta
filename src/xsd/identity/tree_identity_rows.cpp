#include "xsd/internal/tree_identity.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** Целевые элементы области и значения их полей. Строка попадает в
    ** результат, только если есть все поля; у xs:key отсутствие поля —
    ** ошибка (ключ обязателен), у xs:unique и xs:keyref — просто пропуск.
    **
    ** The target elements of a scope and their field values. A row gets into
    ** the result only if all fields are present; for xs:key a missing field is
    ** an error (the key is mandatory), for xs:unique and xs:keyref it is just skipped.
    */
    std::vector<KeyRow> TreeIdentityCheck::selectRows(const IdentityScope& scope, const IdentityConstraint& constraint)
    {
        std::vector<KeyRow> rows;
        xpath::XPathValue selected = constraint.selector.evaluate(*scope.element, constraint.context);

        if (selected.getType() != xpath::ValueType::NODE_SET)
        {
            reportScope(scope, i18n::format(i18n::Message::IDENTITY_SELECTOR_NOT_NODESET, {constraint.name}));
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
    bool TreeIdentityCheck::fieldValue(const Element& target, const xpath::XPathExpression& field,
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

    /*
    ** Тип узла, выбранного полем, — из сведений, записанных при проверке
    ** (аналог PSVI): тип атрибута или простой тип / простое содержимое
    ** элемента. nullptr — тип неизвестен (например, содержимое xs:any с
    ** processContents="skip"), тогда значение сравнивается как строка.
    **
    ** The type of the node selected by a field — from the information recorded during validation
    ** (an analog of the PSVI): the attribute type or the simple type / simple content of
    ** an element. nullptr — the type is unknown (for example, xs:any content with
    ** processContents="skip"), then the value is compared as a string.
    */
    const SimpleType* TreeIdentityCheck::typeOfItem(const xpath::XPathItem& item) const
    {
        if (item.attribute != nullptr)
        {
            auto found = _attributeTypes.find(item.attribute);
            if (found != _attributeTypes.end())
                return found->second;
            return nullptr;
        }
        auto found = _elementTypes.find(static_cast<const Element*>(item.node));
        if (found != _elementTypes.end())
            return found->second;
        return nullptr;
    }
}
