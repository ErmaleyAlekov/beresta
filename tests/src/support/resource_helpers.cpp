#include "support/resource_helpers.hpp"

#include <optional>

namespace support
{
    namespace
    {
        /*
        ** Резолвер по таблице: ищет systemId дословно. URI ресурса равен
        ** его systemId — этого достаточно, пока тесты не проверяют базу.
        **
        ** A table resolver: looks up the systemId verbatim. The resource URI equals
        ** its systemId — this is enough as long as the tests do not check the base.
        */
        class MapResolver : public beresta::ResourceResolver
        {
        public:
            explicit MapResolver(const FileMap& files)
                : _files(files)
            {
            }

            // Неизвестный systemId — «ресурс недоступен»
            // An unknown systemId means "the resource is unavailable"
            std::optional<beresta::Resource> resolve(const beresta::ResourceRequest& request) override
            {
                auto found = _files.find(std::string(request.systemId));

                if (found == _files.end())
                    return std::nullopt;
                return beresta::Resource{found->second, found->first};
            }

        private:
            const FileMap& _files;
        };
    }

    // Фатальная ошибка ловится здесь, ошибки валидности копит CollectingErrorHandler
    // A fatal error is caught here, validity errors are accumulated by CollectingErrorHandler
    ParseOutcome parseWithFiles(const std::string& document, const FileMap& files, bool validate)
    {
        MapResolver resolver(files);
        beresta::CollectingErrorHandler collector;
        beresta::ParserOptions options;
        ParseOutcome outcome{false, 0, std::string()};

        options.resolver = &resolver;
        options.loadExternalDtd = true;
        options.validateDtd = validate;
        options.errorHandler = &collector;
        try
        {
            beresta::Document parsed = beresta::XmlParser(options).parse(document);

            outcome.rootText = parsed.getRoot()->getTextContent();
        }
        catch (const beresta::DomException&)
        {
            outcome.fatal = true;
        }
        for (const beresta::Diagnostic& diagnostic : collector.getDiagnostics())
        {
            if (diagnostic.severity == beresta::ErrorSeverity::RECOVERABLE_ERROR)
                ++outcome.validityErrors;
        }
        return outcome;
    }
}
