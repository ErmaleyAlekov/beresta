#include "core/document_type.hpp"

#include <utility>

namespace beresta
{
    // Разобранная модель DTD
    // The parsed DTD model
    const std::shared_ptr<const dtd::DtdModel>& DocumentType::getModel() const noexcept
    {
        return _model;
    }

    // Модель задаёт парсер после разбора DOCTYPE
    // The parser sets the model after parsing the DOCTYPE
    void DocumentType::setModel(std::shared_ptr<const dtd::DtdModel> model) noexcept
    {
        _model = std::move(model);
    }
}
