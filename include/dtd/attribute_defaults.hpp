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
    // Trims spaces at the edges and collapses inner runs of spaces to one
    std::string collapseWhitespace(std::string_view value);

    // Значение по умолчанию из DTD: раскрытие сущностей и нормализация по типу атрибута
    // The default value from the DTD: entity expansion and normalization by the attribute type
    std::string normalizeDefaultValue(DtdContext& context, const std::string& raw,
        AttributeType type);

    /*
    ** Применение <!ATTLIST> к атрибутам элемента из документа:
    ** - значения атрибутов не-CDATA типов нормализуются (раздел 3.3.3);
    ** - при addDefaults отсутствующие атрибуты с умолчанием (VALUE / FIXED)
    **   добавляются — как это делает Xerces.
    **
    ** Applies <!ATTLIST> to the attributes of an element from the document:
    **  - values of non-CDATA attributes are normalized (section 3.3.3);
    **  - with addDefaults, missing attributes with a default (VALUE / FIXED)
    **    are added — as Xerces does.
    */
    void applyAttributeDeclarations(const DtdModel& model, std::string_view element,
        std::vector<Attribute>& attributes, bool addDefaults);
}

#endif
