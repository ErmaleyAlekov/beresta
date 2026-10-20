#include "xsd/internal/tree_identity.hpp"
#include "xsd/internal/identity_common.hpp"

#include <algorithm>
#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** Атрибуты одного элемента из PSVI: типы корректных, атрибуты без типа
    ** (шаблон skip / lax) и объявления отсутствующих атрибутов по умолчанию.
    ** Номер атрибута в дереве совпадает с номером в событии startElement.
    **
    ** The attributes of one element from the PSVI: the types of the valid ones, the attributes without
    ** a type (a skip / lax wildcard) and the declarations of the absent default attributes.
    ** The attribute number in the tree is the same as the number in the startElement event.
    */
    void TreeIdentityCheck::bindAttributes(const Element& element, std::size_t ordinal)
    {
        const std::vector<Attribute>& attributes = element.getAttributes();
        auto defaults = _psvi.defaultAttributes.find(ordinal);

        if (defaults != _psvi.defaultAttributes.end())
            _defaultAttributes.emplace(&element, &defaults->second);
        if (_psvi.attributeTypes.empty() && _psvi.untypedAttributes.empty())
            return;
        for (std::size_t index = 0; index < attributes.size(); ++index)
        {
            AttributeSlot slot{ordinal, index};
            auto found = _psvi.attributeTypes.find(slot);
            if (found != _psvi.attributeTypes.end())
                _attributeTypes.emplace(&attributes[index], found->second);
            if (_psvi.untypedAttributes.count(slot) != 0)
                _untypedAttributes.insert(&attributes[index]);
        }
    }

    /*
    ** Атрибутов по умолчанию нет в дереве, и XPath их не видит. Поэтому поле
    ** разбирается по грамматике XSD (3.11.6) и его атрибутные варианты
    ** сверяются с записанными объявлениями. Вариант без шагов и ".//"
    ** ("@a") смотрит лишь на сам целевой элемент — обход поддерева не нужен.
    **
    ** Default attributes are not in the tree, and XPath does not see them. So the field
    ** is parsed by the XSD grammar (3.11.6) and its attribute alternatives
    ** are matched against the recorded declarations. An alternative without steps and ".//"
    ** ("@a") looks only at the target element itself — no subtree traversal is needed.
    */
    void TreeIdentityCheck::matchDefaults(const Element& target, const xpath::XPathExpression& field,
        const IdentityConstraint& constraint, DefaultMatch& match)
    {
        bool deep = false;

        if (_defaultAttributes.empty())
            return;
        const std::vector<StreamPath>& paths = fieldPaths(field, constraint);
        for (const StreamPath& path : paths)
            deep = deep || (path.attribute && (path.descendant || !path.steps.empty()));
        const Element* element = &target;
        while (element != nullptr)
        {
            matchDefaultsAt(target, *element, paths, match);
            if (deep)
                element = nextElement(*element, target);
            else
                element = nullptr;
        }
    }

    /*
    ** Атрибуты по умолчанию одного элемента поддерева: имена уровней от
    ** целевого элемента (уровень 0) до этого элемента сверяются с путём так
    ** же, как в потоковой проверке (pathReaches).
    **
    ** The default attributes of one element of the subtree: the level names from
    ** the target element (level 0) to this element are matched against the path in the same way
    ** as in the streaming check (pathReaches).
    */
    void TreeIdentityCheck::matchDefaultsAt(const Element& target, const Element& element,
        const std::vector<StreamPath>& paths, DefaultMatch& match) const
    {
        auto found = _defaultAttributes.find(&element);
        std::vector<LevelName> names;

        if (found == _defaultAttributes.end())
            return;
        for (const Element* current = &element; current != nullptr; current = static_cast<const Element*>(
            current->getParent()))
        {
            names.push_back(LevelName{current->getNamespaceURI().value_or(std::string()),
                std::string(current->getLocalName())});
            if (current == &target)
                break;
        }
        std::reverse(names.begin(), names.end());
        for (const AttributeUse* use : *found->second)
        {
            bool reached = false;
            for (const StreamPath& path : paths)
                reached = reached || (path.attribute && nameMatches(path.attributeTest, use->namespaceUri, use->name)
                    && pathReaches(path, names, 0, names.size() - 1));
            if (reached && ++match.count == 1)
                match = DefaultMatch{1, &element, use};
        }
    }

    // Разобранные варианты поля — один раз за проверку; выражение вне грамматики XSD — пустой список
    // The parsed alternatives of a field — once per check; an expression beyond the XSD grammar — an empty list
    const std::vector<StreamPath>& TreeIdentityCheck::fieldPaths(const xpath::XPathExpression& field,
        const IdentityConstraint& constraint)
    {
        auto found = _fieldPaths.find(&field);
        std::vector<StreamPath> paths;

        if (found != _fieldPaths.end())
            return found->second;
        parseStreamPaths(field.getSource(), constraint.context, paths);
        return _fieldPaths.emplace(&field, std::move(paths)).first->second;
    }
}
