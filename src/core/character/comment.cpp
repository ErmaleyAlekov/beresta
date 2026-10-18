#include "core/comment.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

#include <utility>

namespace beresta
{
    // Значение задаётся через setValue(), чтобы сработала проверка Comment
    // The value is set through setValue() so that the Comment check runs
    Comment::Comment(std::string value)
        : CharacterNode()
    {
        setValue(std::move(value));
    }

    // Тип узла — комментарий
    // The node type is a comment
    NodeType Comment::getType() const noexcept
    {
        return NodeType::COMMENT;
    }

    // Автономная копия комментария
    // A standalone copy of the comment
    std::unique_ptr<Node> Comment::clone() const
    {
        return std::make_unique<Comment>(*this);
    }

    // Содержимое комментария не экранируется — XML этого не предусматривает
    // The comment content is not escaped — XML does not provide for that
    void Comment::serialize(std::string& output) const
    {
        output += "<!--";
        output += getValue();
        output += "-->";
    }

    // Запрещаем последовательности, ломающие синтаксис комментария
    // Forbid sequences that break the comment syntax
    void Comment::validateValue(std::string_view value) const
    {
        if (value.find("--") != std::string_view::npos)
            throw DomException(DomErrorCode::INVALID_CONTENT,
                i18n::format(i18n::Message::COMMENT_DOUBLE_HYPHEN));
        if (!value.empty() && value.back() == '-')
            throw DomException(DomErrorCode::INVALID_CONTENT,
                i18n::format(i18n::Message::COMMENT_TRAILING_HYPHEN));
    }
}
