#include "fuzz/fuzz_input.hpp"

#include <utility>

namespace fuzz
{
    // Последняя часть забирает весь остаток, даже если в нём есть разделители
    // The last part takes all the rest, even if it contains separators
    std::vector<std::string_view> splitParts(std::string_view input, std::size_t maxParts)
    {
        std::vector<std::string_view> parts;

        while (parts.size() + 1 < maxParts)
        {
            std::size_t found = input.find(PART_SEPARATOR);

            if (found == std::string_view::npos)
                break;
            parts.push_back(input.substr(0, found));
            input.remove_prefix(found + PART_SEPARATOR.size());
        }
        parts.push_back(input);
        return parts;
    }

    // Биты флагов включают возможности парсера независимо друг от друга
    // The flag bits enable the parser features independently of each other
    beresta::ParserOptions optionsFromFlags(std::uint8_t flags)
    {
        beresta::ParserOptions options;

        options.namespaceAware = (flags & 0x01) != 0;
        options.validateDtd = (flags & 0x02) != 0;
        options.loadExternalDtd = (flags & 0x04) != 0;
        options.preserveWhitespaceText = (flags & 0x08) != 0;
        options.coalesceCData = (flags & 0x10) != 0;
        options.trackLocations = (flags & 0x20) != 0;
        options.applyDtdDefaults = (flags & 0x40) != 0;
        options.processXInclude = (flags & 0x80) != 0;
        options.maxDepth = 256;
        options.maxEntityExpansions = 2000;
        options.maxExpandedSize = 1024 * 1024;
        return options;
    }

    MemoryResolver::MemoryResolver(std::string_view content)
        : _content(content)
    {
    }

    // Один и тот же текст на любой запрос; URI фиксирован, чтобы база ссылок была стабильной
    // The same text for any request; the URI is fixed so that the reference base is stable
    std::optional<beresta::Resource> MemoryResolver::resolve(const beresta::ResourceRequest& request)
    {
        (void)request;
        return beresta::Resource{_content, "memory:/resource.xml"};
    }
}
