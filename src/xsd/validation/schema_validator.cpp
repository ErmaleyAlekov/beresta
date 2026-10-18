#include "xsd/schema_validator.hpp"
#include "xsd/internal/tree_event_source.hpp"
#include "xsd/validating_handler.hpp"

#include <utility>

namespace beresta::xsd
{
    // Валидатор разделяет неизменяемую схему
    // The validator shares the immutable schema
    SchemaValidator::SchemaValidator(std::shared_ptr<const Schema> schema)
        : _schema(std::move(schema))
    {
    }

    /*
    ** Каждый вызов — новый независимый проход (потокобезопасно). Дерево
    ** превращается в поток событий и проверяется тем же потоковым
    ** валидатором, что и документ при разборе; пространства имён валидатор
    ** разрешает сам по объявлениям xmlns, а ключи проверяет по дереву.
    **
    ** Each call is a new independent pass (thread-safe). The tree
    ** is turned into an event stream and validated by the same streaming
    ** validator as a document during parsing; the validator resolves namespaces
    ** itself by the xmlns declarations, and checks the keys over the tree.
    */
    std::vector<Diagnostic> SchemaValidator::validate(const Document& document) const
    {
        ValidationSettings settings;
        detail::TreeEventSource source;

        settings.resolveNamespaces = true;
        settings.identity = IdentityEvaluation::ON_TREE;
        ValidatingHandler validator(_schema, nullptr, settings);
        source.run(document, validator);
        validator.checkIdentity(document);
        return validator.takeDiagnostics();
    }

    // Документ корректен, если ошибок нет
    // The document is valid if there are no errors
    bool SchemaValidator::isValid(const Document& document) const
    {
        return validate(document).empty();
    }

    // Глобальный тип по имени: сначала типы схемы, затем встроенные типы XSD
    // A global type by name: first the schema types, then the built-in XSD types
    TypeRef Schema::findType(std::string_view namespaceUri, std::string_view localName, bool& found) const
    {
        auto own = _globalTypes.find(Key(namespaceUri, localName));

        found = true;
        if (own != _globalTypes.end())
            return own->second;
        if (namespaceUri == XSD_NAMESPACE_URI && localName == "anyType")
            return TypeRef();
        const SimpleType* builtin = nullptr;
        if (namespaceUri == XSD_NAMESPACE_URI)
            builtin = builtinType(localName);
        found = builtin != nullptr;
        return TypeRef{builtin, nullptr};
    }
}
