#ifndef ATTRIBUTE_DEFAULTS_HPP
#define ATTRIBUTE_DEFAULTS_HPP

#include "core/attribute.hpp"
#include "dtd/dtd_model.hpp"
#include "dtd/dtd_reader.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace beresta::dtd
{
    // Убирает пробелы по краям и сжимает внутренние последовательности пробелов до одного
    std::string collapseWhitespace(std::string_view value);

    // Значение по умолчанию из DTD: раскрытие сущностей и нормализация по типу атрибута
    std::string normalizeDefaultValue(DtdContext& context, const std::string& raw,
        AttributeType type);

    /*
    ** Применение <!ATTLIST> к атрибутам элемента из документа:
    ** - значения атрибутов не-CDATA типов нормализуются (раздел 3.3.3);
    ** - при addDefaults отсутствующие атрибуты с умолчанием (VALUE / FIXED)
    **   добавляются — как это делает Xerces.
    */
    void applyAttributeDeclarations(const DtdModel& model, std::string_view element,
        std::vector<Attribute>& attributes, bool addDefaults);
}

#endif
