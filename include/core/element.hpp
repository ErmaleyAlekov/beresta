#ifndef ELEMENT_HPP
#define ELEMENT_HPP

#include "core/attribute.hpp"
#include "core/container_node.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace beresta
{
    /*
    ** Element — XML-элемент: имя тега, упорядоченный список атрибутов и дети.
    **
    ** Атрибуты хранятся в std::vector, а не в std::map, чтобы при
    ** сериализации сохранялся исходный порядок — вывод совпадает с вводом.
    ** Для типичного количества атрибутов (единицы) линейный поиск
    ** быстрее дерева и не тратит память на узлы (KISS).
    **
    ** Пространства имён вычисляются «вживую» по объявлениям xmlns в самом
    ** элементе и его предках. Поэтому они всегда согласованы с тем, что
    ** будет записано в XML-текст: перенос элемента в другое дерево или
    ** изменение атрибута xmlns сразу отражаются в getNamespaceURI().
    ** Это отличие от Xerces, где URI замораживается в момент создания узла.
    **
    ** Копирование (правило нуля): неявный конструктор копирования вызывает
    ** ContainerNode(const ContainerNode&), который глубоко клонирует детей,
    ** и копирует имя с атрибутами. Копия полностью независима и автономна:
    **     Element copy = original;            // глубокая копия на стеке
    **     auto heap = original.clone();       // глубокая копия через базовый тип
    */
    class Element : public ContainerNode
    {
    public:
        explicit Element(std::string name);

        NodeType getType() const noexcept override;
        std::unique_ptr<Node> clone() const override;
        void serialize(std::string& output) const override;
        bool isEqualNode(const Node& other) const override;

        // Копия только самого элемента: имя и атрибуты, без детей (cloneNode(false) в DOM)
        std::unique_ptr<Element> cloneShallow() const;

        // Заменяет всех детей одним текстовым узлом (пустая строка — просто удаляет детей)
        void setTextContent(std::string text);

        // Дописывает "<name attr="v"" — открывающий тег без закрывающей скобки
        void serializeStartTag(std::string& output) const;

        // ---- Имя ----

        // Полное (квалифицированное) имя тега, например "soap:Envelope"
        const std::string& getName() const noexcept;
        void setName(std::string name);

        // Префикс ("soap") и локальное имя ("Envelope"); без префикса — пустой prefix
        std::string_view getPrefix() const noexcept;
        std::string_view getLocalName() const noexcept;

        // ---- Атрибуты ----

        // Устанавливает значение атрибута (создаёт новый или заменяет существующий)
        void setAttribute(std::string_view name, std::string value);

        // Значение атрибута или std::nullopt, если атрибута нет
        std::optional<std::string> getAttribute(std::string_view name) const;

        // Есть ли атрибут с таким именем
        bool hasAttribute(std::string_view name) const noexcept;

        // Удаляет атрибут; возвращает false, если его не было
        bool removeAttribute(std::string_view name);

        // Все атрибуты в порядке добавления
        const std::vector<Attribute>& getAttributes() const noexcept;

        // ---- Пространства имён ----

        // URI пространства имён элемента или nullopt, если элемент вне пространств имён
        std::optional<std::string> getNamespaceURI() const;

        // URI, связанный с префиксом в этой точке дерева (пустой префикс — пространство по умолчанию)
        std::optional<std::string> lookupNamespaceURI(std::string_view prefix) const;

        // Префикс, связанный с URI в этой точке дерева ("" — пространство по умолчанию)
        std::optional<std::string> lookupPrefix(std::string_view namespaceUri) const;

        // Объявляет пространство имён на этом элементе (xmlns="uri" или xmlns:prefix="uri")
        void declareNamespace(std::string_view prefix, std::string namespaceUri);

        // Атрибут по пространству имён и локальному имени
        std::optional<std::string> getAttributeNS(std::string_view namespaceUri,
            std::string_view localName) const;
        bool hasAttributeNS(std::string_view namespaceUri, std::string_view localName) const;
        bool removeAttributeNS(std::string_view namespaceUri, std::string_view localName);

        // Устанавливает атрибут "prefix:local"; при необходимости объявляет префикс
        void setAttributeNS(std::string_view namespaceUri, std::string_view qualifiedName,
            std::string value);

        // ---- Поиск среди непосредственных детей ----

        // Первый дочерний элемент с указанным именем или nullptr
        Element* findChildElement(std::string_view name);
        const Element* findChildElement(std::string_view name) const;

        // Все дочерние элементы с указанным именем
        std::vector<Element*> findChildElements(std::string_view name);
        std::vector<const Element*> findChildElements(std::string_view name) const;

        // Первый дочерний элемент с указанным пространством имён и локальным именем
        Element* findChildElementNS(std::string_view namespaceUri, std::string_view localName);
        const Element* findChildElementNS(std::string_view namespaceUri,
            std::string_view localName) const;

    protected:
        // Элемент может содержать любые узлы, кроме документа и DOCTYPE
        void validateChild(const Node& child, std::size_t index) const override;

    private:
        // Индекс атрибута по имени или npos
        std::size_t findAttribute(std::string_view name) const noexcept;

        // Индекс атрибута по пространству имён и локальному имени или npos
        std::size_t findAttributeNS(std::string_view namespaceUri,
            std::string_view localName) const;

        // Пространство имён атрибута (у атрибута без префикса пространства нет)
        std::optional<std::string> attributeNamespace(const Attribute& attribute) const;

        // Вывод атрибутов: ровно один пробел перед каждым атрибутом
        void serializeAttributes(std::string& output) const;

        std::string _name;
        std::vector<Attribute> _attributes;
    };
}

#endif
