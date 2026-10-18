#include "dtd/dtd_model.hpp"

namespace beresta::dtd
{
    // Все объявления элементов (для проверок вроде «No Duplicate Types»)
    // All element declarations (for checks like "No Duplicate Types")
    const std::map<std::string, ElementDeclaration, std::less<>>& DtdModel::getElements() const noexcept
    {
        return _elements;
    }

    // Все списки атрибутов по именам элементов (для проверок ID, NOTATION, умолчаний)
    // All attribute lists by element names (for the ID, NOTATION and default checks)
    const std::map<std::string, std::vector<AttributeDeclaration>, std::less<>>&
        DtdModel::getAttributeLists() const noexcept
    {
        return _attributes;
    }

    // Все общие сущности (для проверки нотаций у неразбираемых сущностей)
    // All general entities (to check the notations of unparsed entities)
    const std::map<std::string, EntityDeclaration, std::less<>>& DtdModel::getEntities() const noexcept
    {
        return _entities;
    }
}
