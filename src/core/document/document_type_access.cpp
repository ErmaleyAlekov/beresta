#include "core/document_type.hpp"

#include <utility>

namespace beresta
{
    // Имя корневого элемента
    const std::string& DocumentType::getName() const noexcept
    {
        return _name;
    }

    // Публичный идентификатор
    const std::string& DocumentType::getPublicId() const noexcept
    {
        return _publicId;
    }

    // Системный идентификатор
    const std::string& DocumentType::getSystemId() const noexcept
    {
        return _systemId;
    }

    // Внутреннее подмножество как есть
    const std::string& DocumentType::getInternalSubset() const noexcept
    {
        return _internalSubset;
    }

    // Равенство по всем строковым частям (модель — производная от них)
    bool DocumentType::isEqualNode(const Node& other) const
    {
        if (other.getType() != NodeType::DOCUMENT_TYPE)
            return false;
        const DocumentType& type = static_cast<const DocumentType&>(other);
        if (type._name != _name || type._publicId != _publicId)
            return false;
        return type._systemId == _systemId && type._internalSubset == _internalSubset;
    }
}
