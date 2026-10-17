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
    */
    class CharacterNode : public Node
    {
    public:
        // Текущее значение узла
        const std::string& getValue() const noexcept;

        // Устанавливает новое значение после проверки validateValue()
        void setValue(std::string value);

        // Для символьных узлов текстовое содержимое совпадает со значением
        std::string getTextContent() const override;

        // Узлы равны, если совпадают их тип и значение
        bool isEqualNode(const Node& other) const override;

    protected:
        CharacterNode() noexcept;

        // Крючок проверки значения; по умолчанию разрешено любое значение
        virtual void validateValue(std::string_view value) const;

    private:
        std::string _value;
    };
}

#endif
