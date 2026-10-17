#include "parser/internal/parse_pipeline.hpp"
#include "encoding/encoding_detector.hpp"
#include "parser/error_handler.hpp"
#include "parser/parse_session.hpp"

#include <optional>

namespace beresta::detail
{
    // Перекодированный текст живёт в decoded до конца разбора — сессия хранит лишь представления
    void runParse(std::string_view bytes, const ParserOptions& options, ContentHandler& handler)
    {
        std::optional<encoding::DecodedDocument> decoded;

        try
        {
            decoded.emplace(encoding::decodeDocument(bytes, options.forcedEncoding));
        }
        catch (const ParseException& problem)
        {
            if (options.errorHandler != nullptr)
                options.errorHandler->fatalError(problem);
            throw;
        }
        ParseSession session(decoded->getText(), options, handler);
        session.run();
    }

    // Явно заданный baseUri имеет приоритет над путём к файлу
    ParserOptions optionsForFile(const ParserOptions& options, const std::filesystem::path& path)
    {
        ParserOptions result = options;

        if (result.baseUri.empty())
            result.baseUri = path.u8string();
        return result;
    }
}
