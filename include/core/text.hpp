#ifndef TEXT_HPP
#define TEXT_HPP

#include "core/character_node.hpp"

#include <memory>
#include <string>

namespace beresta
{
    /*
    ** Text — текстовый узел. При сериализации спецсимволы экранируются
    ** (&amp;, &lt;, &gt;), поэтому значение может содержать что угодно.
    ** Копирование и перемещение — неявные (правило нуля): всю работу
    ** выполняют корректные операции базовых классов.
    */
    class Text : public CharacterNode
    {
    public:
        explicit Text(std::string value = "");

        NodeType getType() const noexcept override;
        std::unique_ptr<Node> clone() const override;
        void serialize(std::string& output) const override;
    };
}

#endif
