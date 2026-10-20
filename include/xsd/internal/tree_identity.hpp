#ifndef XSD_TREE_IDENTITY_HPP
#define XSD_TREE_IDENTITY_HPP

#include "xsd/internal/diagnostic_log.hpp"
#include "xsd/internal/identity_path.hpp"
#include "xsd/internal/identity_tracker.hpp"
#include "xsd/internal/path_tracker.hpp"
#include "xpath/xpath.hpp"

#include <cstddef>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace beresta::xsd::detail
{
    // Область ограничений, записанная в потоке: номер элемента, объявление и шаг пути
    // A constraint scope recorded in the stream: the element number, the declaration and the path step
    struct ScopeRecord
    {
        std::size_t ordinal;
        const ElementDecl* declaration;
        std::size_t step;
        EventPosition position;
    };

    // Атрибут в PSVI: номер элемента и номер атрибута в нём
    // An attribute in the PSVI: the element number and the attribute number inside it
    struct AttributeSlot
    {
        std::size_t ordinal;
        std::size_t index;
    };

    bool operator==(const AttributeSlot& left, const AttributeSlot& right) noexcept;

    // Хеш номеров (номера — не данные документа, поэтому секретный ключ не нужен)
    // A hash of numbers (numbers are not document data, so no secret key is needed)
    struct AttributeSlotHash
    {
        std::size_t operator()(const AttributeSlot& slot) const noexcept;
    };

    /*
    ** Сведения проверки (аналог PSVI), нужные ключам по дереву: шаги путей
    ** типы элементов и атрибутов и позиции элементов (дерево может быть
    ** построено без trackLocations), элементы составного типа без простого
    ** содержимого, атрибуты без типа (шаблон skip / lax) и отсутствующие
    ** атрибуты со значением по умолчанию — их в дереве нет, но поля ключей
    ** обязаны их видеть. В потоке узлов дерева ещё нет, поэтому
    ** всё записано по порядковым номерам элементов в прямом порядке; после
    ** постройки дерева номера привязываются к узлам одним обходом.
    **
    ** Validation information (an analog of the PSVI) needed by the keys over a tree: path steps
    ** the types of elements and attributes and the element positions (the tree may be
    ** built without trackLocations), the elements of a complex type without simple
    ** content, the attributes without a type (a skip / lax wildcard) and the absent
    ** attributes with a default value — they are not in the tree, but key fields
    ** must see them. In a stream there are no tree nodes yet, so
    ** everything is recorded by the ordinals of elements in document order; after
    ** the tree is built the numbers are bound to the nodes in one traversal.
    */
    struct TreePsvi
    {
        std::vector<ScopeRecord> scopes;
        std::unordered_map<std::size_t, std::size_t> steps;
        std::unordered_map<std::size_t, EventPosition> locations;
        std::unordered_map<std::size_t, const SimpleType*> elementTypes;
        std::unordered_map<AttributeSlot, const SimpleType*, AttributeSlotHash> attributeTypes;
        std::unordered_set<std::size_t> complexElements;
        std::unordered_set<AttributeSlot, AttributeSlotHash> untypedAttributes;
        std::unordered_map<std::size_t, std::vector<const AttributeUse*>> defaultAttributes;
    };

    /*
    ** TreeIdentityRecorder — стратегия ключей для режимов с деревом
    ** (XmlParser со схемой, SchemaValidator::validate): в потоке только
    ** запись PSVI, проверка — прежним алгоритмом XPath по готовому дереву.
    ** Так любые выражения xs:selector / xs:field работают как раньше.
    **
    ** TreeIdentityRecorder — the key strategy for the modes with a tree
    ** (XmlParser with a schema, SchemaValidator::validate): in the stream only
    ** PSVI recording, the check — with the former XPath algorithm over the ready tree.
    ** This way any xs:selector / xs:field expressions work as before.
    */
    class TreeIdentityRecorder : public IdentityTracker
    {
    public:
        TreeIdentityRecorder(PathTracker& paths, DiagnosticLog& log);

        void openScope(std::size_t level, std::size_t ordinal, const EventPosition& position,
            const ElementDecl& declaration) override;
        bool elementStarted(const IdentityStart& start) override;
        void elementEnded(const IdentityEnd& end) override;
        void finishDocument(const Locator* locator) override;
        void checkTree(const Document& document) override;

    private:
        PathTracker& _paths;
        DiagnosticLog& _log;
        TreePsvi _psvi;
    };

    // Область ограничений в дереве
    // A constraint scope in the tree
    struct IdentityScope
    {
        const Element* element;
        const ElementDecl* declaration;
        std::size_t path;
        SourceLocation location;
    };

    // Строка таблицы ключей: целевой элемент, канонические значения полей и их запись для сообщений
    // A key table row: the target element, the canonical field values and their form for messages
    struct KeyRow
    {
        const Element* target;
        std::vector<std::string> canonical;
        std::vector<std::string> shown;
    };

    // Атрибуты по умолчанию, выбранные полем: сколько их и первый из них с элементом-владельцем
    // The default attributes selected by a field: how many there are and the first of them with the owner element
    struct DefaultMatch
    {
        std::size_t count = 0;
        const Element* owner = nullptr;
        const AttributeUse* use = nullptr;
    };

    // Таблица значений xs:key / xs:unique в одной области
    // The table of xs:key / xs:unique values in one scope
    struct KeyTable
    {
        const Element* scope;
        const IdentityConstraint* constraint;
        std::set<std::vector<std::string>> keys;
    };

    /*
    ** TreeIdentityCheck — проверка xs:key / xs:unique / xs:keyref по
    ** готовому дереву (прежний алгоритм). Сначала строятся таблицы
    ** xs:key / xs:unique во всех областях, затем проверяются xs:keyref —
    ** ссылка может указывать на ключ из области-потомка. Значения ключей
    ** сравниваются по значению типа, а не по строке.
    **
    ** TreeIdentityCheck — checking xs:key / xs:unique / xs:keyref over a
    ** ready tree (the former algorithm). First the xs:key / xs:unique tables
    ** are built in all scopes, then xs:keyref are checked —
    ** a reference may point to a key from a descendant scope. Key values
    ** are compared by the type value, not by the string.
    */
    class TreeIdentityCheck
    {
    public:
        TreeIdentityCheck(const TreePsvi& psvi, DiagnosticLog& log);

        void run(const Document& document);

    private:
        // ---- Привязка PSVI к узлам и сообщения (tree_identity_bind.cpp) ----
        // ---- Binding the PSVI to nodes and messages (tree_identity_bind.cpp) ----
        void bind(const Element& root);
        void bindElement(const Element& element, std::size_t ordinal);
        void reportElement(const Element& element, std::string message);
        void reportScope(const IdentityScope& scope, std::string message);

        // ---- Строки и поля (tree_identity_rows.cpp) ----
        // ---- Rows and fields (tree_identity_rows.cpp) ----
        std::vector<KeyRow> selectRows(const IdentityScope& scope, const IdentityConstraint& constraint);
        bool fieldValue(const Element& target, const xpath::XPathExpression& field,
            const IdentityConstraint& constraint, KeyRow& row);
        bool nodeValue(const Element& target, const xpath::XPathItem& item, const IdentityConstraint& constraint,
            KeyRow& row);
        const SimpleType* typeOfItem(const xpath::XPathItem& item) const;

        // ---- Атрибуты в PSVI и по умолчанию (tree_identity_defaults.cpp) ----
        // ---- Attributes in the PSVI and by default (tree_identity_defaults.cpp) ----
        void bindAttributes(const Element& element, std::size_t ordinal);
        void matchDefaults(const Element& target, const xpath::XPathExpression& field,
            const IdentityConstraint& constraint, DefaultMatch& match);
        void matchDefaultsAt(const Element& target, const Element& element, const std::vector<StreamPath>& paths,
            DefaultMatch& match) const;
        const std::vector<StreamPath>& fieldPaths(const xpath::XPathExpression& field,
            const IdentityConstraint& constraint);

        // ---- Таблицы и ссылки (tree_identity_tables.cpp) ----
        // ---- Tables and references (tree_identity_tables.cpp) ----
        void checkConstraints();
        void buildKeyTable(const IdentityScope& scope, const IdentityConstraint& constraint);
        void checkKeyReferences(const IdentityScope& scope, const IdentityConstraint& keyref);
        bool keyExists(const Element& scope, const IdentityConstraint& key,
            const std::vector<std::string>& values) const;

        const TreePsvi& _psvi;
        DiagnosticLog& _log;
        std::size_t _sequence;
        std::size_t _nextScope;
        std::vector<IdentityScope> _scopes;
        std::vector<KeyTable> _tables;
        std::unordered_map<const Element*, std::size_t> _elementPaths;
        std::unordered_map<const Element*, SourceLocation> _elementLocations;
        std::unordered_map<const Element*, const SimpleType*> _elementTypes;
        std::unordered_map<const Attribute*, const SimpleType*> _attributeTypes;
        std::unordered_set<const Element*> _complexElements;
        std::unordered_set<const Attribute*> _untypedAttributes;
        std::unordered_map<const Element*, const std::vector<const AttributeUse*>*> _defaultAttributes;
        std::unordered_map<const xpath::XPathExpression*, std::vector<StreamPath>> _fieldPaths;
    };
}

#endif
