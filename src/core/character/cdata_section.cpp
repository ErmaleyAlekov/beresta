#include "core/cdata_section.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Значение задаётся через setValue(), чтобы сработала проверка CDATA
    CDataSection::CDataSection(std::string value)
        : CharacterNode()
    {
        setValue(std::move(value));
    }

    // Тип узла — секция CDATA
    NodeType CDataSection::getType() const noexcept
    {
        return NodeType::CDATA_SECTION;
    }

    // Автономная копия секции
    std::unique_ptr<Node> CDataSection::clone() const
    {
        return std::make_unique<CDataSection>(*this);
    }

    // Содержимое выводится «как есть» — в этом смысл CDATA
    void CDataSection::serialize(std::string& output) const
    {
        output += "<![CDATA[";
        output += getValue();
        output += "]]>";
    }

    // Единственное ограничение — отсутствие завершающей последовательности
    void CDataSection::validateValue(std::string_view value) const
    {
        if (value.find("]]>") != std::string_view::npos)
            throw DomException(DomErrorCode::INVALID_CONTENT,
                "секция CDATA не может содержать \"]]>\"");
    }
}
