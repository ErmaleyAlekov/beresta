#include "core/comment.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Значение задаётся через setValue(), чтобы сработала проверка Comment
    Comment::Comment(std::string value)
        : CharacterNode()
    {
        setValue(std::move(value));
    }

    // Тип узла — комментарий
    NodeType Comment::getType() const noexcept
    {
        return NodeType::COMMENT;
    }

    // Автономная копия комментария
    std::unique_ptr<Node> Comment::clone() const
    {
        return std::make_unique<Comment>(*this);
    }

    // Содержимое комментария не экранируется — XML этого не предусматривает
    void Comment::serialize(std::string& output) const
    {
        output += "<!--";
        output += getValue();
        output += "-->";
    }

    // Запрещаем последовательности, ломающие синтаксис комментария
    void Comment::validateValue(std::string_view value) const
    {
        if (value.find("--") != std::string_view::npos)
            throw DomException(DomErrorCode::INVALID_CONTENT,
                "комментарий не может содержать \"--\"");
        if (!value.empty() && value.back() == '-')
            throw DomException(DomErrorCode::INVALID_CONTENT,
                "комментарий не может заканчиваться символом '-'");
    }
}
