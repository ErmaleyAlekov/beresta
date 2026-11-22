#include "bench/engines/xerces_engine.hpp"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/validators/common/Grammar.hpp>

namespace bench
{
    // Счётчик ошибок проверки: ошибки не бросаются, а считаются / A validation error counter: errors are counted, not thrown
    class ErrorCounter : public xercesc::HandlerBase
    {
    public:
        std::size_t count = 0;

        // Ошибка валидности / A validity error
        void error(const xercesc::SAXParseException& exception) override
        {
            (void)exception;
            ++count;
        }

        // Фатальная ошибка разбора / Fatal parse error
        void fatalError(const xercesc::SAXParseException& exception) override
        {
            (void)exception;
            throw BenchError("Xerces: документ отвергнут / document rejected");
        }
    };

    /*
    ** Проверяющий парсер создаётся один раз: схема загружается в кэш
    ** грамматик (loadGrammar), и каждый следующий разбор берёт её оттуда.
    **
    ** The validating parser is created once: the schema is loaded into the grammar
    ** cache (loadGrammar), and every next parse takes it from there.
    */
    static std::unique_ptr<xercesc::XercesDOMParser> makeValidatingParser(const std::string& xsd)
    {
        auto parser = std::make_unique<xercesc::XercesDOMParser>();
        xercesc::MemBufInputSource schema(reinterpret_cast<const XMLByte*>(xsd.data()), xsd.size(), "schema.xsd");

        parser->setDoNamespaces(true);
        parser->setDoSchema(true);
        parser->setValidationScheme(xercesc::XercesDOMParser::Val_Always);
        if (!parser->loadGrammar(schema, xercesc::Grammar::SchemaGrammarType, true) || parser->getErrorCount())
            throw BenchError("Xerces: схема отвергнута / schema rejected");
        parser->useCachedGrammarInParse(true);
        return parser;
    }

    // Разбор с проверкой по закэшированной схеме / Parsing with validation against the cached schema
    std::optional<std::size_t> XercesEngine::parseValidated(const std::string& xml, const std::string& xsd)
    {
        xercesc::MemBufInputSource source(reinterpret_cast<const XMLByte*>(xml.data()), xml.size(), "bench");
        ErrorCounter counter;

        if (_validating == nullptr)
            _validating = makeValidatingParser(xsd);
        _validating->setErrorHandler(&counter);
        try
        {
            _validating->parse(source);
        }
        catch (...)
        {
            _validating.reset();
            throw;
        }
        _validating->setErrorHandler(nullptr);
        _validating->resetDocumentPool();
        return counter.count;
    }
}
