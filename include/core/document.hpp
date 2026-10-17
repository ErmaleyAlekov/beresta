#ifndef DOCUMENT_HPP
#define DOCUMENT_HPP

#include "core/container_node.hpp"
#include "core/document_type.hpp"
#include "core/element.hpp"
#include "core/xml_declaration.hpp"

#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace beresta
{
    /*
    ** Document — лёгкий контейнер верхнего уровня.
    ** В отличие от Xerces, это НЕ глобальный менеджер узлов: он не
    ** создаёт узлы, не владеет пулами памяти и не «привязывает» их к себе.
    ** Он лишь хранит XML-декларацию, пролог (DOCTYPE, комментарии, PI) и ровно
    ** один корневой элемент. Любой узел можно извлечь из документа
    ** через removeChild()/releaseRoot(), и он продолжит жить автономно.
    **
    ** Допустимые дети: не более одного DocumentType (строго до корня),
    ** не более одного Element, а также сколько угодно Comment и PI.
    ** Текст и CDATA на верхнем уровне по стандарту XML запрещены.
    */
    class Document : public ContainerNode
    {
    public:
        Document() noexcept;

        NodeType getType() const noexcept override;
        std::unique_ptr<Node> clone() const override;
        void serialize(std::string& output) const override;
        bool isEqualNode(const Node& other) const override;

        // Корневой элемент или nullptr, если его ещё нет
        Element* getRoot();
        const Element* getRoot() const;

        // Устанавливает корень: заменяет существующий на том же месте или добавляет в конец
        Element* setRoot(std::unique_ptr<Element> root);

        // Извлекает корневой элемент из документа, передавая владение вызывающему
        std::unique_ptr<Element> releaseRoot();

        // XML-декларация (nullopt — документ без декларации)
        const std::optional<XmlDeclaration>& getDeclaration() const noexcept;
        void setDeclaration(std::optional<XmlDeclaration> declaration);

        // Узел DOCTYPE или nullptr
        DocumentType* getDocumentType();
        const DocumentType* getDocumentType() const;

        /*
        ** Элемент по значению ID-атрибута. ID-атрибутами считаются объявленные
        ** в DTD как ID, а также xml:id (как и в Xerces, без DTD обычный
        ** атрибут "id" идентификатором не является).
        */
        Element* getElementById(std::string_view id);
        const Element* getElementById(std::string_view id) const;

    protected:
        // Проверка допустимости типа и позиции ребёнка верхнего уровня
        void validateChild(const Node& child, std::size_t index) const override;

    private:
        std::optional<XmlDeclaration> _declaration;
    };
}

#endif
