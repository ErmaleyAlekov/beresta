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
    **
    ** Document — a lightweight top-level container.
    ** Unlike Xerces, it is NOT a global node manager: it does not
    ** create nodes, does not own memory pools and does not "bind" them to itself.
    ** It only keeps the XML declaration, the prolog (DOCTYPE, comments, PIs) and exactly
    ** one root element. Any node can be extracted from the document
    ** through removeChild()/releaseRoot(), and it keeps living standalone.
    ** Allowed children: at most one DocumentType (strictly before the root),
    ** at most one Element, and any number of Comment and PI.
    ** Text and CDATA at the top level are forbidden by the XML standard.
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
        // The root element or nullptr if there is none yet
        Element* getRoot();
        const Element* getRoot() const;

        // Устанавливает корень: заменяет существующий на том же месте или добавляет в конец
        // Sets the root: replaces the existing one in the same place or appends it to the end
        Element* setRoot(std::unique_ptr<Element> root);

        // Извлекает корневой элемент из документа, передавая владение вызывающему
        // Extracts the root element from the document, passing the ownership to the caller
        std::unique_ptr<Element> releaseRoot();

        // XML-декларация (nullopt — документ без декларации)
        // The XML declaration (nullopt — a document without a declaration)
        const std::optional<XmlDeclaration>& getDeclaration() const noexcept;
        void setDeclaration(std::optional<XmlDeclaration> declaration);

        // Узел DOCTYPE или nullptr
        // The DOCTYPE node or nullptr
        DocumentType* getDocumentType();
        const DocumentType* getDocumentType() const;

        /*
        ** Элемент по значению ID-атрибута. ID-атрибутами считаются объявленные
        ** в DTD как ID, а также xml:id (как и в Xerces, без DTD обычный
        ** атрибут "id" идентификатором не является).
        **
        ** An element by the value of an ID attribute. ID attributes are those declared
        ** as ID in the DTD, and also xml:id (as in Xerces, without a DTD an ordinary
        ** "id" attribute is not an identifier).
        */
        Element* getElementById(std::string_view id);
        const Element* getElementById(std::string_view id) const;

    protected:
        // Проверка допустимости типа и позиции ребёнка верхнего уровня
        // Checks that the type and position of a top-level child are allowed
        void validateChild(const Node& child, std::size_t index) const override;

        // Крючки обхода ContainerNode (document_hooks.cpp): документ без детей
        // ContainerNode traversal hooks (document_hooks.cpp): the document without children
        std::unique_ptr<ContainerNode> cloneEmpty() const override;
        bool equalsShallow(const ContainerNode& other) const override;
        void serializeOpen(std::string& output) const override;
        void serializeClose(std::string& output) const override;

    private:
        std::optional<XmlDeclaration> _declaration;
    };
}

#endif
