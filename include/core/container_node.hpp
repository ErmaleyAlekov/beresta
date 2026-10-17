#ifndef CONTAINER_NODE_HPP
#define CONTAINER_NODE_HPP

#include "core/node.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
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
    */
    class ContainerNode : public Node
    {
    public:
        ~ContainerNode() override;

        /*
        ** Принимаем rvalue-ссылку, а не значение: при ошибке (исключении)
        ** владение узлом остаётся у вызывающего кода и узел не теряется.
        ** Это строгая гарантия безопасности исключений.
        */

        // Добавляет узел в конец списка детей и возвращает невладеющий указатель на него
        Node* appendChild(std::unique_ptr<Node>&& child);

        // Вставляет узел в позицию index (0..getChildCount()) и возвращает указатель на него
        Node* insertChild(std::size_t index, std::unique_ptr<Node>&& child);

        // Отсоединяет ребёнка и возвращает владение им: узел становится автономным
        std::unique_ptr<Node> removeChild(std::size_t index);

        // Отсоединяет указанного ребёнка (по ссылке, а не по индексу)
        std::unique_ptr<Node> detachChild(const Node& child);

        // Заменяет ребёнка по индексу; возвращает владение старым узлом
        std::unique_ptr<Node> replaceChild(std::size_t index, std::unique_ptr<Node>&& child);

        // Удаляет всех детей
        void clearChildren() noexcept;

        // Количество непосредственных детей
        std::size_t getChildCount() const noexcept;

        // Доступ к ребёнку по индексу (при выходе за границы — DomException)
        Node* getChild(std::size_t index);
        const Node* getChild(std::size_t index) const;

        // Первый и последний ребёнок (nullptr, если детей нет)
        Node* getFirstChild();
        const Node* getFirstChild() const;
        Node* getLastChild();
        const Node* getLastChild() const;

        // Индекс непосредственного ребёнка или npos, если это не наш ребёнок
        std::size_t indexOf(const Node* child) const noexcept;

        // Индекс первого ребёнка указанного типа или npos
        std::size_t findFirstOfType(NodeType type) const noexcept;

        // Все элементы поддерева с указанным именем ("*" — любые), в порядке документа
        std::vector<Element*> getElementsByTagName(std::string_view name);
        std::vector<const Element*> getElementsByTagName(std::string_view name) const;

        // Все элементы поддерева с указанным пространством имён и локальным именем ("*" — любые)
        std::vector<Element*> getElementsByTagNameNS(std::string_view namespaceUri,
            std::string_view localName);
        std::vector<const Element*> getElementsByTagNameNS(std::string_view namespaceUri,
            std::string_view localName) const;

        /*
        ** Нормализация (как normalize() в DOM): во всём поддереве соседние
        ** текстовые узлы сливаются в один, а пустые текстовые узлы удаляются.
        */
        void normalize();

        // Конкатенация текста всех потомков (комментарии и PI не учитываются)
        std::string getTextContent() const override;

        // Значение «не найдено» для indexOf()
        static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    protected:
        ContainerNode() noexcept;
        ContainerNode(const ContainerNode& other);
        ContainerNode(ContainerNode&& other) noexcept;
        ContainerNode& operator=(const ContainerNode& other);

        // Не noexcept: бросает DomException при попытке создать цикл владения
        ContainerNode& operator=(ContainerNode&& other);

        // Крючок Template Method: наследник бросает исключение, если тип ребёнка недопустим
        virtual void validateChild(const Node& child, std::size_t index) const = 0;

        // Дописывает сериализацию всех детей подряд, без разделителей
        void serializeChildren(std::string& output) const;

        // Попарное сравнение детей через isEqualNode (для реализаций isEqualNode)
        bool childrenEqual(const ContainerNode& other) const;

    private:
        using ChildList = std::vector<std::unique_ptr<Node>>;

        // Глубоко клонирует список детей другого контейнера
        static ChildList cloneChildren(const ChildList& source);

        // Рекурсивно собирает текст TEXT/CDATA-потомков в output
        static void collectText(const ContainerNode& container, std::string& output);

        // Привязывает всех текущих детей к this (после копирования/перемещения)
        void adoptChildren() noexcept;

        // Обновляет индексы соседей начиная с позиции start
        void reindexFrom(std::size_t start) noexcept;

        // Общие проверки перед вставкой: null, чужой родитель, цикл, тип
        void checkInsertion(const Node* child, std::size_t index) const;

        // Сливает текстовый узел index со следующими текстовыми узлами
        void mergeTextRun(std::size_t index);

        ChildList _children;
    };
}

#endif
