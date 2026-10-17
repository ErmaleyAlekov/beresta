#ifndef PROCESSING_INSTRUCTION_HPP
#define PROCESSING_INSTRUCTION_HPP

#include "core/node.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace beresta
{
    /*
    ** ProcessingInstruction — инструкция обработки <?target data?>.
    ** Цель "xml" (в любом регистре) зарезервирована стандартом: XML-декларация
    ** хранится не как PI, а как свойство документа (Document::getDeclaration).
    ** Между target и data выводится ровно один пробел — и только если data
    ** не пуста: это единственный пробел, которого требует синтаксис.
    */
    class ProcessingInstruction : public Node
    {
    public:
        ProcessingInstruction(std::string target, std::string data = "");

        NodeType getType() const noexcept override;
        std::unique_ptr<Node> clone() const override;
        void serialize(std::string& output) const override;

        // Текстовое содержимое PI — его данные (как в W3C DOM)
        std::string getTextContent() const override;

        // Инструкции равны, если совпадают цель и данные
        bool isEqualNode(const Node& other) const override;

        // Цель инструкции (имя после "<?")
        const std::string& getTarget() const noexcept;

        // Данные инструкции (всё после цели)
        const std::string& getData() const noexcept;

        // Замена данных с проверкой отсутствия "?>"
        void setData(std::string data);

    private:
        // Проверка данных: последовательность "?>" закрыла бы инструкцию досрочно
        static void validateData(std::string_view data);

        std::string _target;
        std::string _data;
    };
}

#endif
