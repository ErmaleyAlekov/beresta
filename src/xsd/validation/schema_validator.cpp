#include "xsd/schema_validator.hpp"
#include "xsd/internal/validation_run.hpp"

#include <utility>

namespace beresta::xsd
{
    // Валидатор разделяет неизменяемую схему
    SchemaValidator::SchemaValidator(std::shared_ptr<const Schema> schema)
        : _schema(std::move(schema))
    {
    }

    // Каждый вызов — новый независимый проход (потокобезопасно)
    std::vector<Diagnostic> SchemaValidator::validate(const Document& document) const
    {
        return detail::ValidationRun(*_schema).run(document);
    }

    // Документ корректен, если ошибок нет
    bool SchemaValidator::isValid(const Document& document) const
    {
        return validate(document).empty();
    }

    // Глобальный тип по имени: сначала типы схемы, затем встроенные типы XSD
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
