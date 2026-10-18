#include "xsd/internal/validation_run.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Проверка xs:keyref в области scope: значение каждой ссылки (если все
    ** её поля есть) обязано найтись среди значений ключа refer. Ключ может
    ** быть объявлен на самом элементе-области или на его потомках — их
    ** таблицы «поднимаются» к области, как требует спецификация XSD.
    **
    ** Checks xs:keyref in the scope scope: the value of each reference (if all
    ** its fields are present) must be found among the values of the refer key. The key may
    ** be declared on the scope element itself or on its descendants — their
    ** tables are "lifted" to the scope, as the XSD specification requires.
    */
    void ValidationRun::checkKeyReferences(const IdentityScope& scope, const IdentityConstraint& keyref)
    {
        if (keyref.referenced == nullptr)
            return;
        for (const KeyRow& row : selectRows(scope, keyref))
        {
            if (!keyExists(*scope.element, *keyref.referenced, row.canonical))
                reportElement(*row.target, i18n::format(i18n::Message::KEYREF_NOT_FOUND, {keyref.name, joinKeyValues(row.shown), keyref.referenced->name}));
        }
    }

    // Есть ли значение в таблицах ключа key, построенных в scope или его потомках
    // Whether the value is in the tables of the key key built in scope or its descendants
    bool ValidationRun::keyExists(const Element& scope, const IdentityConstraint& key,
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
    const SimpleType* ValidationRun::typeOfItem(const xpath::XPathItem& item) const
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
