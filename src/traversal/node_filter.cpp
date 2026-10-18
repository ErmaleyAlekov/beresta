#include "traversal/node_filter.hpp"

namespace beresta
{
    // Виртуальный деструктор интерфейса
    // The virtual destructor of the interface
    NodeFilter::~NodeFilter()
    {
    }

    // Соответствие типа узла биту маски whatToShow
    // Whether the node type matches the whatToShow mask bit
    unsigned showMaskFor(NodeType type) noexcept
    {
        switch (type)
        {
            case NodeType::DOCUMENT:
                return show::DOCUMENTS;
            case NodeType::DOCUMENT_TYPE:
                return show::DOCUMENT_TYPES;
            case NodeType::ELEMENT:
                return show::ELEMENTS;
            case NodeType::TEXT:
                return show::TEXTS;
            case NodeType::CDATA_SECTION:
                return show::CDATA_SECTIONS;
            case NodeType::COMMENT:
                return show::COMMENTS;
            case NodeType::PROCESSING_INSTRUCTION:
                return show::PROCESSING_INSTRUCTIONS;
        }
        return 0;
    }
}
