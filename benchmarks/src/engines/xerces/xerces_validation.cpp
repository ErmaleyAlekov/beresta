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

        // Фатальная ошибка тоже считается, чтобы разбор не прерывался исключением
        // A fatal error is counted too so that parsing is not interrupted by an exception
        void fatalError(const xercesc::SAXParseException& exception) override
        {
            (void)exception;
            ++count;
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
        parser->loadGrammar(schema, xercesc::Grammar::SchemaGrammarType, true);
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
        _validating->parse(source);
        _validating->setErrorHandler(nullptr);
        _validating->resetDocumentPool();
        return counter.count;
    }
}
