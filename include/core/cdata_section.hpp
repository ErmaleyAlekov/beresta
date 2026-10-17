#ifndef CDATA_SECTION_HPP
#define CDATA_SECTION_HPP

#include "core/character_node.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace beresta
{
    /*
    ** CDataSection — секция <![CDATA[значение]]>.
    ** Содержимое выводится без экранирования, поэтому значение не может
    ** содержать завершающую последовательность "]]>".
    */
    class CDataSection : public CharacterNode
    {
    public:
        explicit CDataSection(std::string value = "");

        NodeType getType() const noexcept override;
        std::unique_ptr<Node> clone() const override;
        void serialize(std::string& output) const override;

    protected:
        void validateValue(std::string_view value) const override;
    };
}

#endif
