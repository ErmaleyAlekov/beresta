#ifndef NODE_HPP
#define NODE_HPP

#include "core/node_type.hpp"
#include "core/source_location.hpp"

#include <cstddef>
#include <memory>
#include <string>

namespace beresta
{
    class ContainerNode;
    class Document;

    /*
    ** Node — базовый абстрактный узел дерева (роль Component в паттерне
    ** Composite). Все узлы — элементы, текст, комментарии, документ —
    ** обрабатываются через этот единый интерфейс.
    **
    ** Модель владения памятью (ключевое архитектурное решение):
    ** - родитель ВЛАДЕЕТ детьми через std::unique_ptr<Node>;
    ** - ребёнок НАБЛЮДАЕТ за родителем через невладеющий указатель.
    ** Владение строго направлено сверху вниз, поэтому циклов владения нет
    ** в принципе, а указатель на родителя не может «повиснуть»: ребёнок
    ** физически не переживёт владеющего им родителя.
    ** Вариант shared_ptr + weak_ptr отвергнут сознательно: weak_ptr требует,
    ** чтобы КАЖДЫЙ узел жил в shared_ptr, и тогда обычный конструктор
    ** копирования (Element copy = original;) не смог бы корректно привязать
    ** детей к копии. Нам же нужно, чтобы узел жил автономно — на стеке,
    ** в unique_ptr или в контейнере — без глобального менеджера документа.
    **
    ** Узел помнит свой индекс среди детей родителя, поэтому переход
    ** к соседям (getNextSibling / getPreviousSibling) выполняется за O(1).
    **
    ** Копирование: копия узла всегда АВТОНОМНА (не имеет родителя).
    ** Конструктор копирования и операторы присваивания защищены (protected),
    ** чтобы исключить «срезку» объекта (slicing) через ссылку на базовый класс.
    ** Для полиморфного копирования служит виртуальный метод clone().
    **
    ** Node — the base abstract tree node (the Component role in the Composite
    ** pattern). All nodes — elements, text, comments, the document —
    ** are handled through this single interface.
    ** The memory ownership model (a key architectural decision):
    **  - a parent OWNS its children through std::unique_ptr<Node>;
    **  - a child OBSERVES its parent through a non-owning pointer.
    ** Ownership is strictly directed top-down, so there are no ownership cycles
    ** at all, and the parent pointer cannot "dangle": a child physically
    ** cannot outlive the parent that owns it.
    ** The shared_ptr + weak_ptr variant was rejected deliberately: weak_ptr requires
    ** EVERY node to live in a shared_ptr, and then the ordinary copy constructor
    ** (Element copy = original;) could not bind the children to the copy correctly.
    ** We need a node to live standalone — on the stack, in a unique_ptr
    ** or in a container — without a global document manager.
    ** A node remembers its index among the parent's children, so moving
    ** to the siblings (getNextSibling / getPreviousSibling) takes O(1).
    ** Copying: a copy of a node is always STANDALONE (has no parent).
    ** The copy constructor and the assignment operators are protected
    ** to rule out object slicing through a base class reference.
    ** The virtual clone() method serves polymorphic copying.
    */
    class Node
    {
    public:
        virtual ~Node();

        // Тип узла (для безопасного приведения к конкретному классу)
        // The node type (for a safe cast to the concrete class)
        virtual NodeType getType() const noexcept = 0;

        // Полиморфное глубокое копирование: возвращает автономную копию поддерева
        // Polymorphic deep copying: returns a standalone copy of the subtree
        virtual std::unique_ptr<Node> clone() const = 0;

        // Дописывает XML-представление узла в конец output (без лишних пробелов)
        // Appends the XML representation of the node to the end of output (without extra spaces)
        virtual void serialize(std::string& output) const = 0;

        // Текстовое содержимое узла (для элементов — весь текст потомков)
        // The text content of the node (for elements — all text of the descendants)
        virtual std::string getTextContent() const = 0;

        // Структурное равенство (как isEqualNode в DOM): тип, имена, значения, дети
        // Structural equality (like isEqualNode in DOM): the type, names, values, children
        virtual bool isEqualNode(const Node& other) const = 0;

        // Удобная обёртка над serialize(): возвращает XML строкой
        // A convenient wrapper over serialize(): returns the XML as a string
        std::string toString() const;

        // Истина, если ancestor — один из предков узла (сам узел не считается)
        // True if ancestor is one of the node's ancestors (the node itself does not count)
        bool isDescendantOf(const Node& ancestor) const noexcept;

        // Родительский контейнер или nullptr, если узел автономен
        // The parent container or nullptr if the node is standalone
        ContainerNode* getParent() noexcept;
        const ContainerNode* getParent() const noexcept;

        // Соседи слева и справа (nullptr, если их нет или узел автономен)
        // The siblings on the left and on the right (nullptr if there are none or the node is standalone)
        Node* getNextSibling();
        const Node* getNextSibling() const;
        Node* getPreviousSibling();
        const Node* getPreviousSibling() const;

        // Индекс узла среди детей родителя (у автономного узла — 0); O(1)
        // The node's index among the parent's children (0 for a standalone node); O(1)
        std::size_t getIndexInParent() const noexcept;

        // Документ, в котором находится узел (nullptr, если корень дерева — не документ)
        // The document containing the node (nullptr if the tree root is not a document)
        Document* getOwnerDocument();
        const Document* getOwnerDocument() const;

        // Позиция узла в исходном тексте (если парсер её записал)
        // The node position in the source text (if the parser recorded it)
        const SourceLocation& getSourceLocation() const noexcept;
        void setSourceLocation(SourceLocation location) noexcept;

    protected:
        Node() noexcept;
        Node(const Node& other) noexcept;
        Node(Node&& other) noexcept;
        Node& operator=(const Node& other) noexcept;
        Node& operator=(Node&& other) noexcept;

    private:
        // Связь с родителем меняет только ContainerNode при вставке и удалении
        // Only ContainerNode changes the parent link on insertion and removal
        friend class ContainerNode;
        void setParent(ContainerNode* parent) noexcept;
        void setSiblingIndex(std::size_t index) noexcept;

        ContainerNode* _parent;
        std::size_t _siblingIndex;
        SourceLocation _location;
    };
}

#endif
