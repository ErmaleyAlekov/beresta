#include "dtd/dtd_model.hpp"

namespace beresta::dtd
{
    // Объявлена ли нотация
    // Whether a notation is declared
    bool DtdModel::hasNotation(std::string_view name) const noexcept
    {
        return _notations.find(name) != _notations.end();
    }

    // ID-атрибут — тот, что объявлен в <!ATTLIST> с типом ID
    // An ID attribute is one declared in <!ATTLIST> with the ID type
    bool DtdModel::isIdAttribute(std::string_view element, std::string_view attribute) const noexcept
    {
        const AttributeDeclaration* declaration = findAttribute(element, attribute);

        return declaration != nullptr && declaration->type == AttributeType::ID;
    }
}
