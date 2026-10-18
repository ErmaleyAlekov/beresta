#include "xsd/schema_validator.hpp"
#include "xsd/internal/validation_run.hpp"

#include <utility>

namespace beresta::xsd
{
    // Валидатор разделяет неизменяемую схему
    // The validator shares the immutable schema
    SchemaValidator::SchemaValidator(std::shared_ptr<const Schema> schema)
        : _schema(std::move(schema))
    {
    }

    // Каждый вызов — новый независимый проход (потокобезопасно)
    // Each call is a new independent pass (thread-safe)
    std::vector<Diagnostic> SchemaValidator::validate(const Document& document) const
    {
        return detail::ValidationRun(*_schema).run(document);
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
