#include "core/document_type.hpp"

#include <utility>

namespace beresta
{
    // Разобранная модель DTD
    const std::shared_ptr<const dtd::DtdModel>& DocumentType::getModel() const noexcept
    {
        return _model;
    }

    // Модель задаёт парсер после разбора DOCTYPE
    void DocumentType::setModel(std::shared_ptr<const dtd::DtdModel> model) noexcept
    {
        _model = std::move(model);
    }
}
