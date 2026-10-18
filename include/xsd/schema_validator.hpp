#ifndef XSD_SCHEMA_VALIDATOR_HPP
#define XSD_SCHEMA_VALIDATOR_HPP

#include "core/document.hpp"
#include "parser/error_handler.hpp"
#include "xsd/schema.hpp"

#include <memory>
#include <vector>

namespace beresta::xsd
{
    /*
    ** SchemaValidator — проверка готового DOM по XML Schema.
    ** Дерево обходится без рекурсии и превращается в поток событий для того
    ** же потокового валидатора, что проверяет документ при разборе, — логика
    ** проверки одна. Собирает ВСЕ ошибки за один обход. Каждая ошибка содержит путь
    ** к элементу (/catalog/book[2]/price) и, если документ разобран с
    ** trackLocations, строку и столбец в исходном файле.
    ** Валидатор неизменяем и потокобезопасен.
    **
    ** SchemaValidator — validates a ready DOM against an XML Schema.
    ** The tree is traversed without recursion and turned into an event stream for the same
    ** streaming validator that validates a document during parsing — the validation
    ** logic is one. Collects ALL errors in one traversal. Each error contains the path
    ** to the element (/catalog/book[2]/price) and, if the document was parsed with
    ** trackLocations, the line and column in the source file.
    ** The validator is immutable and thread-safe.
    */
    class SchemaValidator
    {
    public:
        explicit SchemaValidator(std::shared_ptr<const Schema> schema);

        // Все найденные ошибки (пустой список — документ корректен)
        // All errors found (an empty list — the document is valid)
        std::vector<Diagnostic> validate(const Document& document) const;

        // Корректен ли документ
        // Whether the document is valid
        bool isValid(const Document& document) const;

    private:
        std::shared_ptr<const Schema> _schema;
    };
}

#endif
