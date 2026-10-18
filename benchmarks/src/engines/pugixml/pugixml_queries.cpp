#include "bench/engines/pugixml_engine.hpp"

#include <string>

namespace bench
{
    // Писатель pugixml в строку (стандартный способ вывода в память) / A pugixml writer into a string (the standard way to output to memory)
    struct StringWriter : pugi::xml_writer
    {
        std::string output;

        void write(const void* data, std::size_t size) override
        {
            output.append(static_cast<const char*>(data), size);
        }
    };

    // Компактный вывод с декларацией (format_raw) / Compact output with the declaration (format_raw)
    std::optional<std::size_t> PugixmlEngine::serialize() const
    {
        StringWriter writer;

        _document.save(writer, "", pugi::format_raw);
        return writer.output.size();
    }

    // Компиляция и вычисление выражения XPath 1.0 / Compiling and evaluating an XPath 1.0 expression
    std::optional<std::size_t> PugixmlEngine::xpathCount(const std::string& query) const
    {
        return _document.select_nodes(query.c_str()).size();
    }
}
