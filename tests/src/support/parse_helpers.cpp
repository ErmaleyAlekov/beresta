#include "support/parse_helpers.hpp"

namespace support
{
    // Код ошибки вместо исключения — удобно проверять в тестах
    beresta::DomErrorCode errorCodeOf(std::string_view input)
    {
        try
        {
            beresta::XmlParser().parse(input);
        }
        catch (const beresta::ParseException& error)
        {
            return error.getCode();
        }
        return beresta::DomErrorCode::NOT_FOUND;
    }

    // Все ошибки валидности собираются CollectingErrorHandler за один проход
    std::size_t countValidityErrors(const std::string& document)
    {
        beresta::CollectingErrorHandler handler;
        beresta::ParserOptions options;
        options.validateDtd = true;
        options.errorHandler = &handler;

        beresta::XmlParser(options).parse(document);
        return handler.getDiagnostics().size();
    }

    // Встроенный тип ищется по имени, значение проверяется без контекста префиксов
    bool acceptsBuiltin(const char* typeName, const char* value)
    {
        const beresta::xsd::SimpleType* type = beresta::xsd::builtinType(typeName);

        return type != nullptr && !beresta::xsd::validateSimpleValue(*type, value, nullptr).has_value();
    }

    // Схема компилируется из текста, документ разбирается и проверяется
    std::vector<beresta::Diagnostic> schemaErrors(const std::string& xsd, const std::string& xml)
    {
        std::shared_ptr<const beresta::xsd::Schema> schema = beresta::xsd::Schema::fromString(xsd);
        beresta::Document document = beresta::XmlParser().parse(xml);

        return beresta::xsd::SchemaValidator(schema).validate(document);
    }
}
