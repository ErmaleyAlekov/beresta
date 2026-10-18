#include "bench/engines/rapidxml_engine.hpp"

namespace bench
{
    // Название и версия (у RapidXML нет макроса версии) / The name and the version (RapidXML has no version macro)
    std::string RapidxmlEngine::name() const
    {
        return "RapidXML 1.13";
    }

    // Копия входа с нулём на конце и разбор на месте / A copy of the input with a trailing zero and in-situ parsing
    void RapidxmlEngine::parse(const std::string& xml)
    {
        _document.clear();
        _buffer.assign(xml.begin(), xml.end());
        _buffer.push_back('\0');
        try
        {
            _document.parse<0>(_buffer.data());
        }
        catch (const rapidxml::parse_error& error)
        {
            throw BenchError(std::string("RapidXML: ") + error.what());
        }
    }

    // Вклад узла: элемент + атрибуты, текст и CDATA — по одному / The node contribution: an element + attributes, text and CDATA — one each
    static std::size_t weight(const rapidxml::xml_node<>* node)
    {
        std::size_t sum = 1;

        if (node->type() == rapidxml::node_data || node->type() == rapidxml::node_cdata)
            return 1;
        if (node->type() != rapidxml::node_element)
            return 0;
        for (const rapidxml::xml_attribute<>* attribute = node->first_attribute(); attribute;
            attribute = attribute->next_attribute())
            ++sum;
        return sum;
    }

    // Обход по явному стеку / A walk with an explicit stack
    std::size_t RapidxmlEngine::traverse() const
    {
        std::vector<const rapidxml::xml_node<>*> stack{&_document};
        std::size_t sum = 0;

        while (!stack.empty())
        {
            const rapidxml::xml_node<>* node = stack.back();
            stack.pop_back();
            sum += weight(node);
            for (const rapidxml::xml_node<>* child = node->first_node(); child; child = child->next_sibling())
                stack.push_back(child);
        }
        return sum;
    }

    // Освобождение дерева и буфера / Freeing the tree and the buffer
    void RapidxmlEngine::release()
    {
        _document.clear();
        _buffer.clear();
    }
}
