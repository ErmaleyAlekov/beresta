#include "core/cdata_section.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Значение задаётся через setValue(), чтобы сработала проверка CDATA
    // The value is set through setValue() so that the CDATA check runs
    CDataSection::CDataSection(std::string value)
        : CharacterNode()
    {
        setValue(std::move(value));
    }

    // Тип узла — секция CDATA
    // The node type is a CDATA section
    NodeType CDataSection::getType() const noexcept
    {
        return NodeType::CDATA_SECTION;
    }

    // Автономная копия секции
    // A standalone copy of the section
    std::unique_ptr<Node> CDataSection::clone() const
    {
        return std::make_unique<CDataSection>(*this);
    }

    // Содержимое выводится «как есть» — в этом смысл CDATA
    // The content is written "as is" — this is the point of CDATA
    void CDataSection::serialize(std::string& output) const
    {
        output += "<![CDATA[";
        output += getValue();
        output += "]]>";
    }

    // Единственное ограничение — отсутствие завершающей последовательности
    // The only restriction is the absence of the terminating sequence
    void CDataSection::validateValue(std::string_view value) const
    {
        if (value.find("]]>") != std::string_view::npos)
            throw DomException(DomErrorCode::INVALID_CONTENT,
                i18n::format(i18n::Message::CDATA_CONTAINS_END));
    }
}
