#include "dtd/dtd_model.hpp"

namespace beresta::dtd
{
    // Общая сущность (&name;)
    // A general entity (&name;)
    const EntityDeclaration* DtdModel::findEntity(std::string_view name) const noexcept
    {
        auto found = _entities.find(name);

        if (found == _entities.end())
            return nullptr;
        return &found->second;
    }

    // Параметрическая сущность (%name;)
    // A parameter entity (%name;)
    const EntityDeclaration* DtdModel::findParameterEntity(std::string_view name) const noexcept
    {
        auto found = _parameterEntities.find(name);

        if (found == _parameterEntities.end())
            return nullptr;
        return &found->second;
    }

    // Объявление элемента
    // An element declaration
    const ElementDeclaration* DtdModel::findElement(std::string_view name) const noexcept
    {
        auto found = _elements.find(name);

        if (found == _elements.end())
            return nullptr;
        return &found->second;
    }

    // Все объявленные атрибуты элемента
    // All declared attributes of an element
    const std::vector<AttributeDeclaration>* DtdModel::findAttributes(
        std::string_view element) const noexcept
    {
        auto found = _attributes.find(element);

        if (found == _attributes.end())
            return nullptr;
        return &found->second;
    }

    // Объявление конкретного атрибута элемента
    // The declaration of a specific element attribute
    const AttributeDeclaration* DtdModel::findAttribute(std::string_view element,
        std::string_view attribute) const noexcept
    {
        const std::vector<AttributeDeclaration>* list = findAttributes(element);

        if (list == nullptr)
            return nullptr;
        for (const AttributeDeclaration& declaration : *list)
        {
            if (declaration.name == attribute)
                return &declaration;
        }
        return nullptr;
    }
}
