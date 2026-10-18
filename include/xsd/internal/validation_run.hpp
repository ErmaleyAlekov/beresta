#ifndef XSD_VALIDATION_RUN_HPP
#define XSD_VALIDATION_RUN_HPP

#include "parser/error_handler.hpp"
#include "utils/seeded_hash.hpp"
#include "xsd/schema.hpp"

#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace beresta::xsd::detail
{
    /*
    ** ValidationRun — один проход проверки документа по схеме.
    ** Обходит дерево сверху вниз: для каждого элемента определяется тип
    ** (из объявления или xsi:type), затем проверяются атрибуты, модель
    ** содержимого (общим алгоритмом content::matchContent) и текст.
    ** Ошибки не прерывают обход — собирается полный список.
    **
    ** ValidationRun — one pass of validating a document against a schema.
    ** Traverses the tree top-down: for each element the type is determined
    ** (from the declaration or xsi:type), then the attributes, the content model
    ** (with the common content::matchContent algorithm) and the text are checked.
    ** Errors do not stop the traversal — the full list is collected.
    */
    /*
    ** Шаг пути к элементу для сообщений об ошибках. Раньше путь вида
    ** /catalog/item[5]/name строился строкой для КАЖДОГО элемента заранее —
    ** это было главной статьёй выделений памяти при проверке, хотя путь
    ** нужен только при ошибке. Теперь у элемента есть лишь номер шага, а
    ** строка собирается по цепочке родителей в момент сообщения.
    ** ordinal — номер среди одноимённых соседей (с 1); 0 — имя единственное.
    **
    ** A path step to an element for error messages. Previously a path like
    ** /catalog/item[5]/name was built as a string for EVERY element in advance —
    ** that was the main source of memory allocations during validation, although the path
    ** is needed only on an error. Now an element has only a step number, and the
    ** string is assembled along the parent chain at the moment of the message.
    ** ordinal is the number among same-named siblings (from 1); 0 means the name is unique.
    */
    struct PathStep
    {
        std::size_t parent;
        const Element* element;
        std::size_t ordinal;
    };

    // Родитель шага корня: «родителя нет» / The parent of the root step: "there is no parent"
    constexpr std::size_t NO_PARENT_STEP = static_cast<std::size_t>(-1);

    // Отложенная проверка элемента: обход дерева идёт по очереди заданий, без рекурсии
    // A deferred element check: the tree traversal goes through a task queue, without recursion
    struct PendingElement
    {
        const Element* element;
        TypeRef type;
        const ElementDecl* declaration;
        std::size_t path;
    };

    /*
    ** Дочерние элементы проверяемого элемента и их пространства имён.
    ** URI вычисляется подъёмом к корню, поэтому считается один раз и
    ** используется и моделью содержимого, и поиском объявлений.
    **
    ** The child elements of the validated element and their namespaces.
    ** A URI is computed by climbing to the root, so it is computed once and
    ** used both by the content model and by the declaration lookup.
    */
    struct ChildList
    {
        std::vector<const Element*> elements;
        std::vector<std::string> uris;
    };

    // Элемент-область ограничений идентичности (его объявление их содержит)
    // An identity constraint scope element (its declaration contains them)
    struct IdentityScope
    {
        const Element* element;
        const ElementDecl* declaration;
        std::size_t path;
    };

    // Строка таблицы ключей: целевой элемент, канонические значения полей и их запись для сообщений
    // A key table row: the target element, the canonical field values and their form for messages
    struct KeyRow
    {
        const Element* target;
        std::vector<std::string> canonical;
        std::vector<std::string> shown;
    };

    // Таблица значений xs:key / xs:unique в одной области
    // The table of xs:key / xs:unique values in one scope
    struct KeyTable
    {
        const Element* scope;
        const IdentityConstraint* constraint;
        std::set<std::vector<std::string>> keys;
    };

    class ValidationRun
    {
    public:
        explicit ValidationRun(const Schema& schema);

        std::vector<Diagnostic> run(const Document& document);

    private:
        // ---- Очередь заданий (xsd_validation_queue.cpp) ----
        // ---- The task queue (xsd_validation_queue.cpp) ----
        void validateQueued();
        void enqueueChildren(std::vector<PendingElement>& children);
        void reportElement(const Element& element, const std::string& message);
        void reportAt(const Element& element, const std::string& location, const std::string& message);

        // ---- Пути для сообщений (xsd_validation_paths.cpp) ----
        // ---- Paths for messages (xsd_validation_paths.cpp) ----
        std::size_t addChildSteps(std::size_t parent);
        std::string pathText(std::size_t step) const;

        // ---- Ограничения идентичности (xsd_validation_identity.cpp, xsd_validation_keyrefs.cpp) ----
        // ---- Identity constraints (xsd_validation_identity.cpp, xsd_validation_keyrefs.cpp) ----
        void checkIdentityConstraints();
        std::vector<KeyRow> selectRows(const IdentityScope& scope, const IdentityConstraint& constraint);
        bool fieldValue(const Element& target, const xpath::XPathExpression& field,
            const IdentityConstraint& constraint, KeyRow& row);
        void buildKeyTable(const IdentityScope& scope, const IdentityConstraint& constraint);
        void checkKeyReferences(const IdentityScope& scope, const IdentityConstraint& keyref);
        bool keyExists(const Element& scope, const IdentityConstraint& key,
            const std::vector<std::string>& values) const;
        const SimpleType* typeOfItem(const xpath::XPathItem& item) const;

        // ---- Элементы (xsd_validation_run.cpp) ----
        // ---- Elements (xsd_validation_run.cpp) ----
        void validateElement(const Element& element, TypeRef type, const ElementDecl* declaration,
            std::size_t path);
        bool applyXsiType(const Element& element, TypeRef& type, std::size_t path);
        bool isNil(const Element& element, const ElementDecl* declaration, std::size_t path);
        void report(const Element& element, std::size_t path, const std::string& message);

        // ---- Простые значения (xsd_validation_values.cpp) ----
        // ---- Simple values (xsd_validation_values.cpp) ----
        void validateSimpleElement(const Element& element, const SimpleType& type,
            const ElementDecl* declaration, std::size_t path);
        void checkValue(const Element& element, const SimpleType& type, std::string_view value,
            const ElementDecl* declaration, std::size_t path);
        void recordIdentity(const SimpleType& type, std::string_view value, const Element& element,
            std::size_t path);
        void finish(const Element& root);

        // ---- Атрибуты (xsd_validation_attributes.cpp) ----
        // ---- Attributes (xsd_validation_attributes.cpp) ----
        void validateAttributes(const Element& element, const ComplexType& type, std::size_t path);
        void checkAttribute(const Element& element, const AttributeUse& use, const Attribute& attribute,
            std::size_t path);
        void checkRequiredAttributes(const Element& element, const ComplexType& type,
            std::size_t path, std::uint64_t seen);

        // ---- Содержимое (xsd_validation_content.cpp) ----
        // ---- Content (xsd_validation_content.cpp) ----
        void validateContent(const Element& element, const ComplexType& type,
            const ElementDecl* declaration, std::size_t path);
        void checkModel(const Element& element, const ComplexType& type, std::size_t path);
        void validateChildren(const ComplexType& type, std::size_t path);
        const ElementDecl* findDeclaration(const ComplexType& type, const Element& child,
            const std::string& namespaceUri) const;

        const Schema& _schema;
        std::vector<Diagnostic> _diagnostics;
        // Значения ID приходят из документа: хеш с секретным ключом против hash flooding
        // ID values come from the document: a hash with a secret key against hash flooding
        std::unordered_set<std::string, utils::SeededHash> _ids;
        std::vector<std::pair<std::string, std::size_t>> _references;
        std::vector<PendingElement> _pending;
        std::vector<PathStep> _steps;

        // Буферы одного элемента: обход идёт без рекурсии, поэтому их можно
        // переиспользовать, не выделяя память заново для каждого элемента
        // The buffers of one element: the traversal goes without recursion, so they can
        // be reused without allocating memory again for every element
        ChildList _children;
        std::vector<content::ChildKey> _keys;
        std::vector<PendingElement> _tasks;

        // Ограничения идентичности. Пути и типы узлов нужны только им, поэтому
        // записываются, лишь когда найдена хотя бы одна область (_scopes)
        // Identity constraints. Node paths and types are needed only by them, so they
        // are recorded only when at least one scope has been found (_scopes)
        std::vector<IdentityScope> _scopes;
        std::vector<KeyTable> _tables;
        std::unordered_map<const Element*, std::size_t> _elementPaths;
        std::unordered_map<const Element*, const SimpleType*> _elementTypes;
        std::unordered_map<const Attribute*, const SimpleType*> _attributeTypes;
    };

    // Путь к ребёнку: /родитель/имя или /родитель/имя[k], если одноимённых соседей несколько
    // The path to a child: /parent/name or /parent/name[k] if there are several siblings with the same name
    std::string childPath(const std::string& parentPath, const Element& child);

    // Текст элемента: без копии, если это один текстовый узел, иначе в buffer (xsd_validation_text.cpp)
    // The element text: without a copy if it is one text node, otherwise in buffer (xsd_validation_text.cpp)
    std::string_view textOf(const Element& element, std::string& buffer);

    // Есть ли среди детей значимый текст (непробельный TEXT или любая CDATA)
    // Whether there is significant text among the children (non-whitespace TEXT or any CDATA)
    bool hasSignificantText(const Element& element);

    // Абсолютный путь элемента вида /a/b[2]/c (для сообщений об ошибках ключей)
    // The absolute path of an element like /a/b[2]/c (for key error messages)
    std::string pathOf(const Element& element);

    // Значения полей ключа через запятую (для сообщений об ошибках)
    // The key field values separated by commas (for error messages)
    std::string joinKeyValues(const std::vector<std::string>& values);

    // Пространство имён и локальное имя атрибута с учётом префикса
    // The namespace and local name of an attribute taking the prefix into account
    std::pair<std::string, std::string_view> attributeKey(const Element& element, const Attribute& attribute);
}

#endif
