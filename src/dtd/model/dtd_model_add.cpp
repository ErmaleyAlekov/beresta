#include "dtd/dtd_model.hpp"

#include <utility>

namespace beresta::dtd
{
    // Общие и параметрические сущности живут в разных пространствах имён
    bool DtdModel::addEntity(EntityDeclaration declaration)
    {
        std::map<std::string, EntityDeclaration, std::less<>>* target = &_entities;

        if (declaration.isParameter)
            target = &_parameterEntities;
        std::string name = declaration.name;
        return target->emplace(std::move(name), std::move(declaration)).second;
    }

    // Повторное объявление элемента — дубль, который вызывающий может сообщить
    bool DtdModel::addElement(ElementDeclaration declaration)
    {
        std::string name = declaration.name;

        return _elements.emplace(std::move(name), std::move(declaration)).second;
    }

    // Первое объявление атрибута выигрывает (так требует стандарт)
    bool DtdModel::addAttribute(std::string_view element, AttributeDeclaration declaration)
    {
        std::vector<AttributeDeclaration>& list = _attributes[std::string(element)];

        for (const AttributeDeclaration& existing : list)
        {
            if (existing.name == declaration.name)
                return false;
        }
        list.push_back(std::move(declaration));
        return true;
    }

    // Нотации нужны только для проверки атрибутов типа NOTATION и сущностей NDATA
    void DtdModel::addNotation(std::string name)
    {
        _notations.insert(std::move(name));
    }

    // Изменяемый поиск сущности (общей или параметрической)
    EntityDeclaration* DtdModel::findEntityForUpdate(std::string_view name, bool parameter) noexcept
    {
        std::map<std::string, EntityDeclaration, std::less<>>* source = &_entities;

        if (parameter)
            source = &_parameterEntities;
        auto found = source->find(name);
        if (found == source->end())
            return nullptr;
        return &found->second;
    }
}
