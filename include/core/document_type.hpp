#ifndef DOCUMENT_TYPE_HPP
#define DOCUMENT_TYPE_HPP

#include "core/node.hpp"

#include <memory>
#include <string>

namespace beresta::dtd
{
    class DtdModel;
}

namespace beresta
{
    /*
    ** DocumentType — объявление <!DOCTYPE name PUBLIC "pub" "sys" [подмножество]>.
    **
    ** Хранит имя корневого элемента, внешние идентификаторы и дословный
    ** текст внутреннего подмножества, чтобы при сериализации DOCTYPE
    ** воспроизводился без потерь. Разобранная модель DTD (объявления
    ** сущностей, элементов и атрибутов) хранится отдельно в неизменяемом
    ** DtdModel; копии узла разделяют одну модель через shared_ptr —
    ** она никогда не меняется, поэтому разделение безопасно.
    */
    class DocumentType : public Node
    {
    public:
        explicit DocumentType(std::string name, std::string publicId = "",
            std::string systemId = "", std::string internalSubset = "");

        NodeType getType() const noexcept override;
        std::unique_ptr<Node> clone() const override;
        void serialize(std::string& output) const override;
        std::string getTextContent() const override;
        bool isEqualNode(const Node& other) const override;

        // Имя корневого элемента из объявления
        const std::string& getName() const noexcept;

        // Публичный идентификатор (PUBLIC "...") или пустая строка
        const std::string& getPublicId() const noexcept;

        // Системный идентификатор — обычно путь к внешнему DTD
        const std::string& getSystemId() const noexcept;

        // Дословный текст между '[' и ']'
        const std::string& getInternalSubset() const noexcept;

        // Разобранная модель DTD (nullptr, если DTD не разбирался)
        const std::shared_ptr<const dtd::DtdModel>& getModel() const noexcept;
        void setModel(std::shared_ptr<const dtd::DtdModel> model) noexcept;

    private:
        std::string _name;
        std::string _publicId;
        std::string _systemId;
        std::string _internalSubset;
        std::shared_ptr<const dtd::DtdModel> _model;
    };
}

#endif
