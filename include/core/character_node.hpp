#ifndef CHARACTER_NODE_HPP
#define CHARACTER_NODE_HPP

#include "core/node.hpp"

#include <string>
#include <string_view>

namespace beresta
{
    /*
    ** CharacterNode — общий предок узлов, хранящих только строковое
    ** значение: Text, Comment, CDataSection (роль Leaf в паттерне Composite).
    ** Хранение и изменение значения реализованы один раз здесь (DRY),
    ** а правила допустимости значения наследник задаёт в validateValue().
    ** Значение хранится в «сыром» виде (UTF-8, без XML-экранирования).
    **
    ** CharacterNode — the common ancestor of nodes that store only a string
    ** value: Text, Comment, CDataSection (the Leaf role in the Composite pattern).
    ** Storing and changing the value are implemented once here (DRY),
    ** and the subclass defines the value validity rules in validateValue().
    ** The value is stored "raw" (UTF-8, without XML escaping).
    */
    class CharacterNode : public Node
    {
    public:
        // Текущее значение узла
        // The current value of the node
        const std::string& getValue() const noexcept;

        // Устанавливает новое значение после проверки validateValue()
        // Sets a new value after the validateValue() check
        void setValue(std::string value);

        // Дописывает кусок в конец значения за O(длина куска) с той же проверкой validateValue():
        // проверяется кусок вместе с двумя последними байтами прежнего значения, потому что
        // запрещённые сочетания ("--", "]]>") короче трёх байтов и могут лечь на стык.
        // Appends a piece to the end of the value in O(piece length) with the same validateValue() check:
        // the piece is checked together with the last two bytes of the former value, because
        // the forbidden combinations ("--", "]]>") are at most three bytes long and may fall on the joint.
        void appendValue(std::string_view piece);

        // Для символьных узлов текстовое содержимое совпадает со значением
        // For character nodes the text content is the same as the value
        std::string getTextContent() const override;

        // Узлы равны, если совпадают их тип и значение
        // Nodes are equal if their type and value match
        bool isEqualNode(const Node& other) const override;

    protected:
        CharacterNode() noexcept;

        // Крючок проверки значения; по умолчанию разрешено любое значение
        // The value check hook; by default any value is allowed
        virtual void validateValue(std::string_view value) const;

    private:
        std::string _value;
    };
}

#endif
