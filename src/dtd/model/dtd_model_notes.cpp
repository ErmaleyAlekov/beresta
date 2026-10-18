#include "dtd/dtd_model.hpp"

#include <utility>

namespace beresta::dtd
{
    // Замечание копится в модели: на этапе разбора DTD ещё неизвестно, проверяют ли валидность
    // The note is accumulated in the model: while the DTD is parsed it is not yet known whether validity is checked
    void DtdModel::addValidityNote(std::string message)
    {
        _validityNotes.push_back(std::move(message));
    }

    // Замечания в порядке обнаружения
    // The notes in the order they were found
    const std::vector<std::string>& DtdModel::getValidityNotes() const noexcept
    {
        return _validityNotes;
    }

    // Отмечается при внешнем подмножестве в DOCTYPE и при любой ссылке %pe;
    // Marked for an external subset in the DOCTYPE and for any %pe; reference
    void DtdModel::markExternalParts() noexcept
    {
        _hasExternalParts = true;
    }

    // Были ли внешнее подмножество или ссылки на параметрические сущности
    // Whether there were an external subset or parameter entity references
    bool DtdModel::hasExternalParts() const noexcept
    {
        return _hasExternalParts;
    }
}
