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
    **
    ** ProcessingInstruction — the <?target data?> processing instruction.
    ** The "xml" target (in any case) is reserved by the standard: the XML declaration
    ** is stored not as a PI but as a document property (Document::getDeclaration).
    ** Exactly one space is written between target and data — and only if data
    ** is not empty: it is the only space the syntax requires.
    */
    class ProcessingInstruction : public Node
    {
    public:
        ProcessingInstruction(std::string target, std::string data = "");

        NodeType getType() const noexcept override;
        std::unique_ptr<Node> clone() const override;
        void serialize(std::string& output) const override;

        // Текстовое содержимое PI — его данные (как в W3C DOM)
        // The text content of a PI is its data (as in W3C DOM)
        std::string getTextContent() const override;

        // Инструкции равны, если совпадают цель и данные
        // Instructions are equal if the target and the data match
        bool isEqualNode(const Node& other) const override;

        // Цель инструкции (имя после "<?")
        // The instruction target (the name after "<?")
        const std::string& getTarget() const noexcept;

        // Данные инструкции (всё после цели)
        // The instruction data (everything after the target)
        const std::string& getData() const noexcept;

        // Замена данных с проверкой отсутствия "?>"
        // Replaces the data, checking that there is no "?>"
        void setData(std::string data);

    private:
        // Проверка данных: последовательность "?>" закрыла бы инструкцию досрочно
        // Checks the data: the "?>" sequence would close the instruction early
        static void validateData(std::string_view data);

        std::string _target;
        std::string _data;
    };
}

#endif
