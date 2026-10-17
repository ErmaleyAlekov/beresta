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
    ** Собирает ВСЕ ошибки за один обход. Каждая ошибка содержит путь
    ** к элементу (/catalog/book[2]/price) и, если документ разобран с
    ** trackLocations, строку и столбец в исходном файле.
    ** Валидатор неизменяем и потокобезопасен.
    */
    class SchemaValidator
    {
    public:
        explicit SchemaValidator(std::shared_ptr<const Schema> schema);

        // Все найденные ошибки (пустой список — документ корректен)
        std::vector<Diagnostic> validate(const Document& document) const;

        // Корректен ли документ
        bool isValid(const Document& document) const;

    private:
        std::shared_ptr<const Schema> _schema;
    };
}

#endif
