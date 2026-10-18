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
    **
    ** Element — an XML element: the tag name, an ordered list of attributes and children.
    ** Attributes are kept in a std::vector, not in a std::map, so that the original
    ** order is preserved on serialization — the output matches the input.
    ** For a typical number of attributes (a few) a linear search is faster
    ** than a tree and spends no memory on nodes (KISS).
    ** Namespaces are computed "live" from the xmlns declarations in the element
    ** itself and its ancestors. That is why they are always consistent with what
    ** will be written to the XML text: moving an element to another tree or
    ** changing an xmlns attribute is immediately reflected in getNamespaceURI().
    ** This differs from Xerces, where the URI is frozen when the node is created.
    ** Copying (the rule of zero): the implicit copy constructor calls
    ** ContainerNode(const ContainerNode&), which deep-clones the children,
    ** and copies the name with the attributes. The copy is fully independent and standalone:
    **     Element copy = original;            // a deep copy on the stack
    **     auto heap = original.clone();       // a deep copy through the base type
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
        // A copy of the element only: the name and attributes, without children (cloneNode(false) in DOM)
        std::unique_ptr<Element> cloneShallow() const;

        // Заменяет всех детей одним текстовым узлом (пустая строка — просто удаляет детей)
        // Replaces all children with one text node (an empty string just removes the children)
        void setTextContent(std::string text);

        // Дописывает "<name attr="v"" — открывающий тег без закрывающей скобки
        // Appends "<name attr="v"" — the start tag without the closing bracket
        void serializeStartTag(std::string& output) const;

        // ---- Имя ----
        // ---- Name ----

        // Полное (квалифицированное) имя тега, например "soap:Envelope"
        // The full (qualified) tag name, for example "soap:Envelope"
        const std::string& getName() const noexcept;
        void setName(std::string name);

        // Префикс ("soap") и локальное имя ("Envelope"); без префикса — пустой prefix
        // The prefix ("soap") and the local name ("Envelope"); without a prefix — an empty prefix
        std::string_view getPrefix() const noexcept;
        std::string_view getLocalName() const noexcept;

        // ---- Атрибуты ----
        // ---- Attributes ----

        // Устанавливает значение атрибута (создаёт новый или заменяет существующий)
        // Sets an attribute value (creates a new one or replaces the existing one)
        void setAttribute(std::string_view name, std::string value);

        // Значение атрибута или std::nullopt, если атрибута нет
        // The attribute value or std::nullopt if there is no such attribute
        std::optional<std::string> getAttribute(std::string_view name) const;

        // Есть ли атрибут с таким именем
        // Whether there is an attribute with this name
        bool hasAttribute(std::string_view name) const noexcept;

        // Удаляет атрибут; возвращает false, если его не было
        // Removes an attribute; returns false if it did not exist
        bool removeAttribute(std::string_view name);

        // Все атрибуты в порядке добавления
        // All attributes in the order they were added
        const std::vector<Attribute>& getAttributes() const noexcept;

        // Резерв места под count атрибутов: построитель дерева выделяет память один раз
        // Reserves room for count attributes: the tree builder allocates memory once
        void reserveAttributes(std::size_t count);

        /*
        ** Пакетная установка атрибутов — то же, что setAttribute для каждой
        ** пары по порядку (повтор имени заменяет значение), но за O(n), а не
        ** O(n²). Все имена проверяются до изменений: при неверном имени
        ** элемент остаётся прежним. Так строится дерево при разборе.
        **
        ** Batch setting of attributes — the same as setAttribute for every
        ** pair in order (a repeated name replaces the value), but in O(n) rather than
        ** O(n²). All names are checked before any change: with an invalid name
        ** the element stays unchanged. This is how the tree is built during parsing.
        */
        void setAttributes(std::vector<Attribute> attributes);

        // ---- Пространства имён ----
        // ---- Namespaces ----

        // URI пространства имён элемента или nullopt, если элемент вне пространств имён
        // The namespace URI of the element or nullopt if the element is in no namespace
        std::optional<std::string> getNamespaceURI() const;

        // URI, связанный с префиксом в этой точке дерева (пустой префикс — пространство по умолчанию)
        // The URI bound to a prefix at this point of the tree (an empty prefix — the default namespace)
        std::optional<std::string> lookupNamespaceURI(std::string_view prefix) const;

        // Префикс, связанный с URI в этой точке дерева ("" — пространство по умолчанию)
        // The prefix bound to a URI at this point of the tree ("" — the default namespace)
        std::optional<std::string> lookupPrefix(std::string_view namespaceUri) const;

        // Объявляет пространство имён на этом элементе (xmlns="uri" или xmlns:prefix="uri")
        // Declares a namespace on this element (xmlns="uri" or xmlns:prefix="uri")
        void declareNamespace(std::string_view prefix, std::string namespaceUri);

        // Атрибут по пространству имён и локальному имени
        // An attribute by namespace and local name
        std::optional<std::string> getAttributeNS(std::string_view namespaceUri,
            std::string_view localName) const;
        bool hasAttributeNS(std::string_view namespaceUri, std::string_view localName) const;
        bool removeAttributeNS(std::string_view namespaceUri, std::string_view localName);

        // Устанавливает атрибут "prefix:local"; при необходимости объявляет префикс
        // Sets the "prefix:local" attribute; declares the prefix if needed
        void setAttributeNS(std::string_view namespaceUri, std::string_view qualifiedName,
            std::string value);

        // ---- Поиск среди непосредственных детей ----
        // ---- Search among direct children ----

        // Первый дочерний элемент с указанным именем или nullptr
        // The first child element with the given name or nullptr
        Element* findChildElement(std::string_view name);
        const Element* findChildElement(std::string_view name) const;

        // Все дочерние элементы с указанным именем
        // All child elements with the given name
        std::vector<Element*> findChildElements(std::string_view name);
        std::vector<const Element*> findChildElements(std::string_view name) const;

        // Первый дочерний элемент с указанным пространством имён и локальным именем
        // The first child element with the given namespace and local name
        Element* findChildElementNS(std::string_view namespaceUri, std::string_view localName);
        const Element* findChildElementNS(std::string_view namespaceUri,
            std::string_view localName) const;

    protected:
        // Элемент может содержать любые узлы, кроме документа и DOCTYPE
        // An element can contain any nodes except a document and a DOCTYPE
        void validateChild(const Node& child, std::size_t index) const override;

        // Крючки обхода ContainerNode (element_hooks.cpp): элемент без детей
        // ContainerNode traversal hooks (element_hooks.cpp): the element without children
        std::unique_ptr<ContainerNode> cloneEmpty() const override;
        bool equalsShallow(const ContainerNode& other) const override;
        void serializeOpen(std::string& output) const override;
        void serializeClose(std::string& output) const override;

    private:
        // Индекс атрибута по имени или npos
        // The attribute index by name or npos
        std::size_t findAttribute(std::string_view name) const noexcept;

        // Индекс атрибута по пространству имён и локальному имени или npos
        // The attribute index by namespace and local name or npos
        std::size_t findAttributeNS(std::string_view namespaceUri,
            std::string_view localName) const;

        // Пространство имён атрибута (у атрибута без префикса пространства нет)
        // The namespace of an attribute (an attribute without a prefix has no namespace)
        std::optional<std::string> attributeNamespace(const Attribute& attribute) const;

        // Вывод атрибутов: ровно один пробел перед каждым атрибутом
        // Writes the attributes: exactly one space before each attribute
        void serializeAttributes(std::string& output) const;

        // Слияние с уже имеющимися атрибутами через индекс имён (element_attributes_bulk.cpp)
        // Merging with the already present attributes through a name index (element_attributes_bulk.cpp)
        void mergeAttributes(std::vector<Attribute>& attributes);

        std::string _name;
        std::vector<Attribute> _attributes;
    };
}

#endif
