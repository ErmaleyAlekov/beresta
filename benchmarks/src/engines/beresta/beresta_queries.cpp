#include "bench/engines/beresta_engine.hpp"

namespace bench
{
    // Компактный вывод в UTF-8 / Compact output in UTF-8
    std::optional<std::size_t> BerestaEngine::serialize() const
    {
        return beresta::XmlSerializer().toString(*_document).size();
    }

    // Компиляция и вычисление выражения (как у остальных — без кэша) / Compiling and evaluating the expression (like the others — without a cache)
    std::optional<std::size_t> BerestaEngine::xpathCount(const std::string& query) const
    {
        beresta::xpath::XPathExpression expression(query);

        return expression.evaluate(*_document).getNodes().size();
    }

    /*
    ** Разбор с проверкой по схеме. Схема компилируется один раз и дальше
    ** переиспользуется (так же кэшируют её libxml2 и Xerces): замеряется
    ** проверка документа, а не загрузка XSD. Ошибки собираются, а не бросаются.
    **
    ** Parsing with schema validation. The schema is compiled once and reused
    ** afterwards (libxml2 and Xerces cache it the same way): what is measured is
    ** the document validation, not loading the XSD. Errors are collected, not thrown.
    */
    std::optional<std::size_t> BerestaEngine::parseValidated(const std::string& xml, const std::string& xsd)
    {
        beresta::ParserOptions options;
        beresta::CollectingErrorHandler handler;

        if (_schema == nullptr)
            _schema = beresta::xsd::Schema::fromString(xsd);
        options.schema = _schema;
        options.errorHandler = &handler;
        beresta::Document document = beresta::XmlParser(options).parse(xml);
        return handler.getDiagnostics().size();
    }

    /*
    ** Потоковая проверка: SaxParser со схемой, дерево не строится — память
    ** проверки O(глубины документа). Обработчик событий пустой: замеряется
    ** разбор плюс проверка, как у libxml2 с xmlSchemaValidateStream.
    **
    ** Streaming validation: SaxParser with a schema, no tree is built — the validation
    ** memory is O(document depth). The event handler is empty: what is measured is
    ** parsing plus validation, as for libxml2 with xmlSchemaValidateStream.
    */
    std::optional<std::size_t> BerestaEngine::streamValidated(const std::string& xml, const std::string& xsd)
    {
        beresta::ParserOptions options;
        beresta::CollectingErrorHandler handler;
        beresta::ContentHandler ignore;

        if (_schema == nullptr)
            _schema = beresta::xsd::Schema::fromString(xsd);
        options.schema = _schema;
        options.errorHandler = &handler;
        beresta::SaxParser(options).parse(xml, ignore);
        return handler.getDiagnostics().size();
    }
}
