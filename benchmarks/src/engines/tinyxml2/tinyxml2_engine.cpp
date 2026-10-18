#include "bench/engines/tinyxml2_engine.hpp"

#include <vector>

namespace bench
{
    // Название и версия / The name and the version
    std::string Tinyxml2Engine::name() const
    {
        return "tinyxml2 " + std::to_string(TINYXML2_MAJOR_VERSION) + "." + std::to_string(TINYXML2_MINOR_VERSION);
    }

    // Разбор буфера (tinyxml2 копирует вход) / Parsing a buffer (tinyxml2 copies the input)
    void Tinyxml2Engine::parse(const std::string& xml)
    {
        if (_document.Parse(xml.data(), xml.size()) != tinyxml2::XML_SUCCESS)
            throw BenchError("tinyxml2: документ отвергнут / the document is rejected");
    }

    // Вклад узла: элемент + атрибуты, текст и CDATA — по одному / The node contribution: an element + attributes, text and CDATA — one each
    static std::size_t weight(const tinyxml2::XMLNode* node)
    {
        const tinyxml2::XMLElement* element = node->ToElement();
        std::size_t sum = 1;

        if (node->ToText() != nullptr)
            return 1;
        if (element == nullptr)
            return 0;
        for (const tinyxml2::XMLAttribute* attribute = element->FirstAttribute(); attribute; attribute = attribute->Next())
            ++sum;
        return sum;
    }

    // Обход по явному стеку / A walk with an explicit stack
    std::size_t Tinyxml2Engine::traverse() const
    {
        std::vector<const tinyxml2::XMLNode*> stack{&_document};
        std::size_t sum = 0;

        while (!stack.empty())
        {
            const tinyxml2::XMLNode* node = stack.back();
            stack.pop_back();
            sum += weight(node);
            for (const tinyxml2::XMLNode* child = node->FirstChild(); child; child = child->NextSibling())
                stack.push_back(child);
        }
        return sum;
    }

    // Освобождение дерева / Freeing the tree
    void Tinyxml2Engine::release()
    {
        _document.Clear();
    }
}
