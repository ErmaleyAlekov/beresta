#include "bench/engines/rapidxml_engine.hpp"

#include <rapidxml/rapidxml_print.hpp>

#include <iterator>
#include <string>

namespace bench
{
    // Компактный вывод в строку (print_no_indenting) / Compact output into a string (print_no_indenting)
    std::optional<std::size_t> RapidxmlEngine::serialize() const
    {
        std::string output;

        rapidxml::print(std::back_inserter(output), _document, rapidxml::print_no_indenting);
        return output.size();
    }
}
