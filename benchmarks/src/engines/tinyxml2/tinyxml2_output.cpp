#include "bench/engines/tinyxml2_engine.hpp"

namespace bench
{
    // Компактный вывод в память (XMLPrinter без отступов) / Compact output to memory (an XMLPrinter without indentation)
    std::optional<std::size_t> Tinyxml2Engine::serialize() const
    {
        tinyxml2::XMLPrinter printer(nullptr, true);

        _document.Print(&printer);
        return static_cast<std::size_t>(printer.CStrSize() - 1);
    }
}
