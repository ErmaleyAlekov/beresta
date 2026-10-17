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
    */
    class Node
    {
    public:
        virtual ~Node();

        // Тип узла (для безопасного приведения к конкретному классу)
        virtual NodeType getType() const noexcept = 0;

        // Полиморфное глубокое копирование: возвращает автономную копию поддерева
        virtual std::unique_ptr<Node> clone() const = 0;

        // Дописывает XML-представление узла в конец output (без лишних пробелов)
        virtual void serialize(std::string& output) const = 0;

        // Текстовое содержимое узла (для элементов — весь текст потомков)
        virtual std::string getTextContent() const = 0;

        // Структурное равенство (как isEqualNode в DOM): тип, имена, значения, дети
        virtual bool isEqualNode(const Node& other) const = 0;

        // Удобная обёртка над serialize(): возвращает XML строкой
        std::string toString() const;

        // Истина, если ancestor — один из предков узла (сам узел не считается)
        bool isDescendantOf(const Node& ancestor) const noexcept;

        // Родительский контейнер или nullptr, если узел автономен
        ContainerNode* getParent() noexcept;
        const ContainerNode* getParent() const noexcept;

        // Соседи слева и справа (nullptr, если их нет или узел автономен)
        Node* getNextSibling();
        const Node* getNextSibling() const;
        Node* getPreviousSibling();
        const Node* getPreviousSibling() const;

        // Индекс узла среди детей родителя (у автономного узла — 0); O(1)
        std::size_t getIndexInParent() const noexcept;

        // Документ, в котором находится узел (nullptr, если корень дерева — не документ)
        Document* getOwnerDocument();
        const Document* getOwnerDocument() const;

        // Позиция узла в исходном тексте (если парсер её записал)
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
        friend class ContainerNode;
        void setParent(ContainerNode* parent) noexcept;
        void setSiblingIndex(std::size_t index) noexcept;

        ContainerNode* _parent;
        std::size_t _siblingIndex;
        SourceLocation _location;
    };
}

#endif
