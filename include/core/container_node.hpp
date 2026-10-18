#ifndef CONTAINER_NODE_HPP
#define CONTAINER_NODE_HPP

#include "core/node.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace beresta
{
    class Element;

    /*
    ** ContainerNode — узел, способный иметь детей (роль Composite в паттерне
    ** Composite). Общая логика управления детьми вынесена сюда, чтобы
    ** Element и Document не дублировали её (DRY).
    **
    ** Вставка построена по паттерну Template Method: общий алгоритм
    ** проверок (пустой указатель, уже имеющийся родитель, цикл) находится
    ** в checkInsertion(), а правила допустимых типов детей наследник
    ** задаёт через виртуальный «крючок» validateChild().
    **
    ** Копирование ГЛУБОКОЕ: копируется всё поддерево через clone() детей.
    **
    ** Ни одна операция над поддеревом не рекурсивна: копирование, сравнение,
    ** сериализация, сбор текста и нормализация идут по явному стеку, а
    ** деструктор разбирает дерево по указателям на родителя вообще без
    ** выделения памяти. Глубина дерева ограничена только объёмом памяти,
    ** а не размером стека вызовов. Наследник описывает лишь «себя без детей»
    ** через крючки cloneEmpty / equalsShallow / serializeOpen / serializeClose
    ** (паттерн Template Method: обход — здесь, детали узла — в наследнике).
    **
    ** ContainerNode — a node that can have children (the Composite role in the
    ** Composite pattern). The common child management logic is moved here so that
    ** Element and Document do not duplicate it (DRY).
    ** Insertion follows the Template Method pattern: the common check algorithm
    ** (a null pointer, an existing parent, a cycle) is in checkInsertion(),
    ** and the subclass defines the allowed child types through the virtual
    ** validateChild() hook.
    ** Copying is DEEP: the whole subtree is copied through clone() of the children.
    ** No subtree operation is recursive: copying, comparison, serialization,
    ** text collection and normalization go through an explicit stack, and the
    ** destructor takes the tree apart through parent pointers without any memory
    ** allocation at all. The tree depth is limited only by the amount of memory,
    ** not by the call stack size. The subclass describes only "itself without children"
    ** through the hooks cloneEmpty / equalsShallow / serializeOpen / serializeClose
    ** (the Template Method pattern: the traversal is here, the node details are in the subclass).
    */
    class ContainerNode : public Node
    {
    public:
        ~ContainerNode() override;

        /*
        ** Принимаем rvalue-ссылку, а не значение: при ошибке (исключении)
        ** владение узлом остаётся у вызывающего кода и узел не теряется.
        ** Это строгая гарантия безопасности исключений.
        **
        ** An rvalue reference is taken, not a value: on an error (an exception)
        ** the ownership of the node stays with the caller and the node is not lost.
        ** This is the strong exception safety guarantee.
        */

        // Добавляет узел в конец списка детей и возвращает невладеющий указатель на него
        // Appends a node to the end of the child list and returns a non-owning pointer to it
        Node* appendChild(std::unique_ptr<Node>&& child);

        // Вставляет узел в позицию index (0..getChildCount()) и возвращает указатель на него
        // Inserts a node at position index (0..getChildCount()) and returns a pointer to it
        Node* insertChild(std::size_t index, std::unique_ptr<Node>&& child);

        // Отсоединяет ребёнка и возвращает владение им: узел становится автономным
        // Detaches a child and returns its ownership: the node becomes standalone
        std::unique_ptr<Node> removeChild(std::size_t index);

        // Отсоединяет указанного ребёнка (по ссылке, а не по индексу)
        // Detaches the given child (by reference, not by index)
        std::unique_ptr<Node> detachChild(const Node& child);

        // Заменяет ребёнка по индексу; возвращает владение старым узлом
        // Replaces a child by index; returns the ownership of the old node
        std::unique_ptr<Node> replaceChild(std::size_t index, std::unique_ptr<Node>&& child);

        // Удаляет всех детей
        // Removes all children
        void clearChildren() noexcept;

        // Количество непосредственных детей
        // The number of direct children
        std::size_t getChildCount() const noexcept;

        // Доступ к ребёнку по индексу (при выходе за границы — DomException)
        // Access to a child by index (out of range — DomException)
        Node* getChild(std::size_t index);
        const Node* getChild(std::size_t index) const;

        // Первый и последний ребёнок (nullptr, если детей нет)
        // The first and the last child (nullptr if there are no children)
        Node* getFirstChild();
        const Node* getFirstChild() const;
        Node* getLastChild();
        const Node* getLastChild() const;

        // Индекс непосредственного ребёнка или npos, если это не наш ребёнок
        // The index of a direct child or npos if it is not our child
        std::size_t indexOf(const Node* child) const noexcept;

        // Индекс первого ребёнка указанного типа или npos
        // The index of the first child of the given type or npos
        std::size_t findFirstOfType(NodeType type) const noexcept;

        // Все элементы поддерева с указанным именем ("*" — любые), в порядке документа
        // All subtree elements with the given name ("*" — any), in document order
        std::vector<Element*> getElementsByTagName(std::string_view name);
        std::vector<const Element*> getElementsByTagName(std::string_view name) const;

        // Все элементы поддерева с указанным пространством имён и локальным именем ("*" — любые)
        // All subtree elements with the given namespace and local name ("*" — any)
        std::vector<Element*> getElementsByTagNameNS(std::string_view namespaceUri,
            std::string_view localName);
        std::vector<const Element*> getElementsByTagNameNS(std::string_view namespaceUri,
            std::string_view localName) const;

        /*
        ** Нормализация (как normalize() в DOM): во всём поддереве соседние
        ** текстовые узлы сливаются в один, а пустые текстовые узлы удаляются.
        **
        ** Normalization (like normalize() in DOM): in the whole subtree adjacent
        ** text nodes are merged into one, and empty text nodes are removed.
        */
        void normalize();

        // Конкатенация текста всех потомков (комментарии и PI не учитываются)
        // The concatenated text of all descendants (comments and PIs are not counted)
        std::string getTextContent() const override;

        // Значение «не найдено» для indexOf()
        // The "not found" value for indexOf()
        static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    protected:
        ContainerNode() noexcept;
        ContainerNode(const ContainerNode& other);
        ContainerNode(ContainerNode&& other) noexcept;
        ContainerNode& operator=(const ContainerNode& other);

        // Не noexcept: бросает DomException при попытке создать цикл владения
        // Not noexcept: throws DomException on an attempt to create an ownership cycle
        ContainerNode& operator=(ContainerNode&& other);

        // Крючок Template Method: наследник бросает исключение, если тип ребёнка недопустим
        // The Template Method hook: the subclass throws an exception if the child type is not allowed
        virtual void validateChild(const Node& child, std::size_t index) const = 0;

        // Дописывает сериализацию всех детей подряд, без разделителей
        // Appends the serialization of all children in a row, without separators
        void serializeChildren(std::string& output) const;

        // Попарное сравнение всего поддерева детей (без рекурсии)
        // Pairwise comparison of the whole child subtree (without recursion)
        bool childrenEqual(const ContainerNode& other) const;

        // ---- Крючки обхода: узел «сам по себе», без детей ----
        // ---- Traversal hooks: the node "by itself", without children ----

        // Копия узла без детей (имя, атрибуты, декларация...)
        // A copy of the node without children (the name, attributes, declaration...)
        virtual std::unique_ptr<ContainerNode> cloneEmpty() const = 0;

        // Равенство собственных свойств; other гарантированно того же типа
        // Equality of own properties; other is guaranteed to be of the same type
        virtual bool equalsShallow(const ContainerNode& other) const = 0;

        // Начало и конец разметки узла, у которого есть дети
        // The start and the end of the markup of a node that has children
        virtual void serializeOpen(std::string& output) const = 0;
        virtual void serializeClose(std::string& output) const = 0;

    private:
        using ChildList = std::vector<std::unique_ptr<Node>>;

        // Отложенное копирование: (оригинал, его пустая копия, ждущая детей)
        // Deferred copying: (the original, its empty copy waiting for children)
        using CopyTask = std::pair<const ContainerNode*, ContainerNode*>;

        // Глубоко клонирует детей другого контейнера (явный стек вместо рекурсии)
        // Deep-clones the children of another container (an explicit stack instead of recursion)
        static ChildList cloneChildren(const ContainerNode& source);

        // Копирует детей source в target; контейнеры — пустыми, с задачей в pending
        // Copies the children of source into target; containers are copied empty, with a task in pending
        static void appendCopies(const ContainerNode& source, ChildList& target,
            std::vector<CopyTask>& pending);

        // Собирает текст TEXT/CDATA-потомков в output (без рекурсии)
        // Collects the text of TEXT/CDATA descendants into output (without recursion)
        static void collectText(const ContainerNode& container, std::string& output);

        // Узел как контейнер (элемент или документ) или nullptr, если это лист
        // The node as a container (an element or a document) or nullptr if it is a leaf
        static const ContainerNode* asContainer(const Node& node) noexcept;
        static ContainerNode* asContainer(Node& node) noexcept;

        // Привязывает всех текущих детей к this (после копирования/перемещения)
        // Binds all current children to this (after copying/moving)
        void adoptChildren() noexcept;

        // Обновляет индексы соседей начиная с позиции start
        // Updates the sibling indexes starting from position start
        void reindexFrom(std::size_t start) noexcept;

        // Общие проверки перед вставкой: null, чужой родитель, цикл, тип
        // Common checks before insertion: null, a foreign parent, a cycle, the type
        void checkInsertion(const Node* child, std::size_t index) const;

        // Сливает текстовый узел index со следующими текстовыми узлами
        // Merges the text node index with the following text nodes
        void mergeTextRun(std::size_t index);

        ChildList _children;
    };
}

#endif
