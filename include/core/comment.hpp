#ifndef COMMENT_HPP
#define COMMENT_HPP

#include "core/character_node.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace beresta
{
    /*
    ** Comment — комментарий <!--значение-->.
    ** По стандарту XML значение не может содержать "--" и не может
    ** заканчиваться на '-', иначе вывод станет некорректным XML.
    */
    class Comment : public CharacterNode
    {
    public:
        explicit Comment(std::string value = "");

        NodeType getType() const noexcept override;
        std::unique_ptr<Node> clone() const override;
        void serialize(std::string& output) const override;

    protected:
        void validateValue(std::string_view value) const override;
    };
}

#endif
