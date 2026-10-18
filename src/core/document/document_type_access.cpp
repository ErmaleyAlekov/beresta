#include "core/document_type.hpp"

#include <utility>

namespace beresta
{
    // Имя корневого элемента
    // The root element name
    const std::string& DocumentType::getName() const noexcept
    {
        return _name;
    }

    // Публичный идентификатор
    // The public identifier
    const std::string& DocumentType::getPublicId() const noexcept
    {
        return _publicId;
    }

    // Системный идентификатор
    // The system identifier
    const std::string& DocumentType::getSystemId() const noexcept
    {
        return _systemId;
    }

    // Внутреннее подмножество как есть
    // The internal subset as is
    const std::string& DocumentType::getInternalSubset() const noexcept
    {
        return _internalSubset;
    }

    // Равенство по всем строковым частям (модель — производная от них)
    // Equality by all string parts (the model is derived from them)
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
