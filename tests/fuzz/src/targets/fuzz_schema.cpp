#include "fuzz/fuzz_input.hpp"

#include "beresta.hpp"

#include <memory>

/*
** Цель фаззинга: загрузка XML Schema (Schema::fromString) и проверка
** документа по ней — отдельным SchemaValidator, встроенной в парсер
** проверкой (ParserOptions::schema) и потоковой проверкой SaxParser.
**
** Формат входа: [байт флагов][схема] 0xF5 0xF5 [документ] 0xF5 0xF5 [ресурс].
** Ресурс отдаётся резолвером на xs:include / xs:import, поэтому фаззер
** мутирует и подключаемые схемы. Флаг 0x01 включает namespaceAware=false
** для документа (схема всегда читается с пространствами имён).
**
** The fuzzing target: loading an XML Schema (Schema::fromString) and validating
** a document against it — with a separate SchemaValidator, with the validation
** built into the parser (ParserOptions::schema) and with streaming SaxParser validation.
** The input format: [flag byte][schema] 0xF5 0xF5 [document] 0xF5 0xF5 [resource].
** The resolver returns the resource for xs:include / xs:import, so the fuzzer
** mutates the included schemas too. The 0x01 flag enables namespaceAware=false
** for the document (the schema is always read with namespaces).
*/
namespace
{
    // Схема или nullptr, если текст схемы некорректен
    // The schema or nullptr if the schema text is invalid
    std::shared_ptr<const beresta::xsd::Schema> loadSchema(std::string_view text,
        beresta::ResourceResolver& resolver)
    {
        beresta::xsd::SchemaLoadOptions options;

        options.resolver = &resolver;
        options.baseUri = "memory:/schema.xsd";
        try
        {
            return beresta::xsd::Schema::fromString(text, options);
        }
        catch (const beresta::DomException&)
        {
            return nullptr;
        }
    }

    // Проверка уже разобранного документа отдельным валидатором
    // Validates an already parsed document with a separate validator
    void validateSeparately(const std::shared_ptr<const beresta::xsd::Schema>& schema,
        std::string_view documentText, bool namespaceAware)
    {
        beresta::ParserOptions options;

        options.namespaceAware = namespaceAware;
        try
        {
            beresta::Document document = beresta::XmlParser(options).parse(documentText);

            (void)beresta::xsd::SchemaValidator(schema).validate(document);
        }
        catch (const beresta::DomException&)
        {
        }
    }

    // Проверка схемой прямо во время разбора; ошибки собираются, а не бросаются
    // Schema validation right during parsing; errors are collected, not thrown
    void validateWhileParsing(const std::shared_ptr<const beresta::xsd::Schema>& schema,
        std::string_view documentText)
    {
        beresta::CollectingErrorHandler collector;
        beresta::ParserOptions options;

        options.schema = schema;
        options.errorHandler = &collector;
        try
        {
            (void)beresta::XmlParser(options).parse(documentText);
        }
        catch (const beresta::DomException&)
        {
        }
    }

    /*
    ** Потоковая проверка без дерева: SaxParser со схемой (ключи — потоковым
    ** вычислителем). Флаг пространств имён тот же, что у отдельной проверки:
    ** без них пары «URI + имя» вычисляет сам валидатор.
    **
    ** Streaming validation without a tree: SaxParser with a schema (keys — by the streaming
    ** evaluator). The namespace flag is the same as for the separate check:
    ** without them the validator computes the "URI + name" pairs itself.
    */
    void validateWithSax(const std::shared_ptr<const beresta::xsd::Schema>& schema,
        std::string_view documentText, bool namespaceAware)
    {
        beresta::CollectingErrorHandler collector;
        beresta::ContentHandler ignore;
        beresta::ParserOptions options;

        options.schema = schema;
        options.errorHandler = &collector;
        options.namespaceAware = namespaceAware;
        try
        {
            beresta::SaxParser(options).parse(documentText, ignore);
        }
        catch (const beresta::DomException&)
        {
        }
    }
}

// Точка входа libFuzzer
// The libFuzzer entry point
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    if (size == 0)
        return 0;
    std::string_view input(reinterpret_cast<const char*>(data + 1), size - 1);
    std::vector<std::string_view> parts = fuzz::splitParts(input, 3);

    while (parts.size() < 3)
        parts.push_back(std::string_view());
    fuzz::MemoryResolver resolver(parts[2]);
    std::shared_ptr<const beresta::xsd::Schema> schema = loadSchema(parts[0], resolver);

    if (schema == nullptr)
        return 0;
    validateSeparately(schema, parts[1], (data[0] & 0x01) == 0);
    validateWhileParsing(schema, parts[1]);
    validateWithSax(schema, parts[1], (data[0] & 0x01) == 0);
    return 0;
}
