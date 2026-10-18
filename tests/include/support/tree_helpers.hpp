#ifndef TREE_HELPERS_HPP
#define TREE_HELPERS_HPP

#include "beresta.hpp"

#include <cstddef>

namespace support
{
    // Документ-цепочка <a><a>...<a>x</a>...</a></a> заданной глубины (строится сверху вниз)
    // A chain document <a><a>...<a>x</a>...</a></a> of the given depth (built top-down)
    beresta::Document deepChain(std::size_t depth);

    // Самый глубокий элемент цепочки
    // The deepest element of the chain
    beresta::Element* deepestElement(beresta::Document& document);
}

#endif
