#include "bench/engines/pugixml_engine.hpp"

#include <vector>

namespace bench
{
    // Название и версия (PUGIXML_VERSION = 1140 для 1.14) / The name and the version (PUGIXML_VERSION = 1140 for 1.14)
    std::string PugixmlEngine::name() const
    {
        return "pugixml " + std::to_string(PUGIXML_VERSION / 1000) + "." + std::to_string(PUGIXML_VERSION % 1000 / 10);
    }

    // Разбор буфера (load_buffer копирует вход) / Parsing a buffer (load_buffer copies the input)
    void PugixmlEngine::parse(const std::string& xml)
    {
        if (!_document.load_buffer(xml.data(), xml.size()))
            throw BenchError("pugixml: документ отвергнут / the document is rejected");
    }

    // Вклад узла: элемент + атрибуты, текст и CDATA — по одному / The node contribution: an element + attributes, text and CDATA — one each
    static std::size_t weight(const pugi::xml_node& node)
    {
        std::size_t sum = 1;

        if (node.type() == pugi::node_pcdata || node.type() == pugi::node_cdata)
            return 1;
        if (node.type() != pugi::node_element)
            return 0;
        for (pugi::xml_attribute attribute = node.first_attribute(); attribute; attribute = attribute.next_attribute())
            ++sum;
        return sum;
    }

    // Обход по явному стеку / A walk with an explicit stack
    std::size_t PugixmlEngine::traverse() const
    {
        std::vector<pugi::xml_node> stack{_document};
        std::size_t sum = 0;

        while (!stack.empty())
        {
            pugi::xml_node node = stack.back();
            stack.pop_back();
            sum += weight(node);
            for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling())
                stack.push_back(child);
        }
        return sum;
    }

    // Освобождение дерева / Freeing the tree
    void PugixmlEngine::release()
    {
        _document.reset();
    }
}
