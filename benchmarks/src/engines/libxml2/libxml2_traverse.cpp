#include "bench/engines/libxml2_engine.hpp"

#include <vector>

namespace bench
{
    // Вклад узла: элемент + атрибуты, текст и CDATA — по одному / The node contribution: an element + attributes, text and CDATA — one each
    static std::size_t weight(xmlNodePtr node)
    {
        std::size_t sum = 0;

        if (node->type == XML_TEXT_NODE || node->type == XML_CDATA_SECTION_NODE)
            return 1;
        if (node->type != XML_ELEMENT_NODE)
            return 0;
        for (xmlAttrPtr attribute = node->properties; attribute != nullptr; attribute = attribute->next)
            ++sum;
        return sum + 1;
    }

    // Обход по явному стеку: дети — список children / next / A walk with an explicit stack: the children are the children / next list
    std::size_t Libxml2Engine::traverse() const
    {
        std::vector<xmlNodePtr> stack;
        std::size_t sum = 0;

        for (xmlNodePtr child = _document->children; child != nullptr; child = child->next)
            stack.push_back(child);
        while (!stack.empty())
        {
            xmlNodePtr node = stack.back();
            stack.pop_back();
            sum += weight(node);
            if (node->type != XML_ELEMENT_NODE)
                continue;
            for (xmlNodePtr child = node->children; child != nullptr; child = child->next)
                stack.push_back(child);
        }
        return sum;
    }
}
