#include "support/tree_helpers.hpp"

namespace support
{
    // Цепочка <a><a>...<a>x</a>...</a></a> заданной глубины, построенная сверху вниз
    // A chain <a><a>...<a>x</a>...</a></a> of the given depth, built top-down
    beresta::Document deepChain(std::size_t depth)
    {
        beresta::Document document;
        beresta::Element* cursor = document.setRoot(std::make_unique<beresta::Element>("a"));

        for (std::size_t level = 1; level < depth; ++level)
            cursor = static_cast<beresta::Element*>(cursor->appendChild(std::make_unique<beresta::Element>("a")));
        cursor->appendChild(std::make_unique<beresta::Text>("x"));
        return document;
    }

    // Самый глубокий элемент цепочки
    // The deepest element of the chain
    beresta::Element* deepestElement(beresta::Document& document)
    {
        beresta::Element* cursor = document.getRoot();

        while (cursor->getFirstChild() != nullptr && cursor->getFirstChild()->getType() == beresta::NodeType::ELEMENT)
            cursor = static_cast<beresta::Element*>(cursor->getFirstChild());
        return cursor;
    }
}
