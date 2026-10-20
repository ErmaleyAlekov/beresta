#ifndef XSD_STREAM_IDENTITY_HPP
#define XSD_STREAM_IDENTITY_HPP

#include "xsd/internal/diagnostic_log.hpp"
#include "xsd/internal/identity_path.hpp"
#include "xsd/internal/identity_tracker.hpp"
#include "xsd/internal/path_tracker.hpp"

#include <cstddef>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace beresta::xsd::detail
{
    // Ограничение, разобранное для потока: варианты селектора и каждого поля
    // A constraint parsed for the stream: the alternatives of the selector and of each field
    struct CompiledConstraint
    {
        bool streamable = false;
        std::vector<StreamPath> selector;
        std::vector<std::vector<StreamPath>> fields;
    };

    /*
    ** Состояние поля у целевого элемента: сколько разных узлов выбрано
    ** (больше одного — ошибка), откуда значение (атрибут — сразу, элемент —
    ** при его закрытии), само значение и его канонический вид (считается
    ** сразу: префиксы QName действуют лишь внутри элемента). complex —
    ** элемент не простого типа, untyped — атрибут без типа: оба ошибочны.
    **
    ** The state of a field at a target element: how many different nodes are selected
    ** (more than one is an error), where the value comes from (an attribute — at once, an element —
    ** when it is closed), the value itself and its canonical form (computed
    ** at once: QName prefixes are in effect only inside the element). complex —
    ** an element not of a simple type, untyped — an attribute without a type: both are erroneous.
    */
    struct FieldState
    {
        std::size_t matches = 0;
        bool fromElement = false;
        std::size_t elementOrdinal = 0;
        bool complex = false;
        bool untyped = false;
        std::string value;
        std::string canonical;
    };

    // Целевой элемент селектора, у которого ещё собираются поля
    // A target element of a selector whose fields are still being collected
    struct ActiveTarget
    {
        std::size_t scope = 0;
        std::size_t constraint = 0;
        std::size_t level = 0;
        std::size_t ordinal = 0;
        std::size_t step = 0;
        EventPosition position;
        std::vector<FieldState> fields;
    };

    // Полная строка ключа: значения полей и место целевого элемента для сообщений
    // A complete key row: the field values and the place of the target element for messages
    struct StreamRow
    {
        std::size_t ordinal;
        std::size_t step;
        EventPosition position;
        std::vector<std::string> canonical;
        std::vector<std::string> shown;
    };

    // Ограничение в открытой области: его номер в объявлении и собранные строки
    // A constraint in an open scope: its number in the declaration and the collected rows
    struct ScopeConstraint
    {
        const IdentityConstraint* constraint;
        const CompiledConstraint* compiled;
        std::size_t index;
        std::vector<StreamRow> rows;
    };

    // Открытая область ограничений идентичности
    // An open scope of identity constraints
    struct StreamScope
    {
        std::size_t level;
        std::size_t ordinal;
        std::vector<ScopeConstraint> constraints;
    };

    // Готовая таблица xs:key / xs:unique: диапазон номеров элементов области и значения
    // A finished xs:key / xs:unique table: the range of the scope element numbers and the values
    struct StreamTable
    {
        const IdentityConstraint* constraint;
        std::size_t first;
        std::size_t last;
        std::set<std::vector<std::string>> keys;
    };

    /*
    ** StreamIdentity — потоковая проверка xs:key / xs:unique / xs:keyref
    ** для режима SAX без дерева (как у Xerces). Пути селекторов и полей —
    ** из грамматики XSD (child, attribute, ".//"), поэтому их можно
    ** сопоставлять со стеком имён открытых элементов. Строки ключей
    ** собираются к закрытию целевого элемента, таблицы и ссылки — к
    ** закрытию области. Порядок сообщений, их пути и тексты — как у
    ** проверки по дереву.
    **
    ** StreamIdentity — streaming checking of xs:key / xs:unique / xs:keyref
    ** for the SAX mode without a tree (as in Xerces). The selector and field paths are
    ** from the XSD grammar (child, attribute, ".//"), so they can
    ** be matched against the stack of names of the open elements. Key rows
    ** are collected by the closing of the target element, tables and references — by the
    ** closing of the scope. The order of messages, their paths and texts are the same as in
    ** the check over a tree.
    */
    class StreamIdentity : public IdentityTracker
    {
    public:
        StreamIdentity(PathTracker& paths, DiagnosticLog& log);

        void openScope(std::size_t level, std::size_t ordinal, const EventPosition& position,
            const ElementDecl& declaration) override;
        bool elementStarted(const IdentityStart& start) override;
        void elementEnded(const IdentityEnd& end) override;
        void finishDocument(const Locator* locator) override;
        void checkTree(const Document& document) override;

    private:
        // ---- Селекторы и поля (stream_identity_match.cpp) ----
        // ---- Selectors and fields (stream_identity_match.cpp) ----
        const CompiledConstraint& compile(const IdentityConstraint& constraint);
        void selectTargets(const IdentityStart& start);
        bool matchFields(ActiveTarget& target, const IdentityStart& start);

        // ---- Атрибуты в полях (stream_identity_fields.cpp) ----
        // ---- Attributes in fields (stream_identity_fields.cpp) ----
        void matchAttributes(FieldState& field, const std::vector<StreamPath>& paths,
            const ActiveTarget& target, const IdentityStart& start);
        void matchDefaults(FieldState& field, const std::vector<StreamPath>& paths,
            const ActiveTarget& target, const IdentityStart& start);

        // ---- Строки ключей (stream_identity_rows.cpp) ----
        // ---- Key rows (stream_identity_rows.cpp) ----
        void captureElement(const IdentityEnd& end);
        void finishTarget(const ActiveTarget& target);
        bool fieldRowValue(const ActiveTarget& target, std::size_t index, StreamRow& row);
        void reportTarget(const ActiveTarget& target, std::size_t sub, std::string message);

        // ---- Таблицы и ссылки (stream_identity_tables.cpp) ----
        // ---- Tables and references (stream_identity_tables.cpp) ----
        void closeScope(StreamScope& scope);
        void buildTable(const StreamScope& scope, ScopeConstraint& entry);
        void resolveReferences(const StreamScope& scope, ScopeConstraint& entry);
        bool keyExists(const StreamScope& scope, const IdentityConstraint& key,
            const std::vector<std::string>& values) const;

        PathTracker& _paths;
        DiagnosticLog& _log;
        std::unordered_map<const IdentityConstraint*, CompiledConstraint> _compiled;
        std::unordered_set<const IdentityConstraint*> _reportedUnstreamable;
        std::vector<LevelName> _names;
        std::vector<StreamScope> _scopes;
        std::vector<ActiveTarget> _targets;
        std::size_t _targetCount;
        std::vector<StreamTable> _tables;
        std::size_t _lastOrdinal;
    };
}

#endif
