#include "bench/engines/xerces_engine.hpp"

#include <xercesc/framework/MemBufFormatTarget.hpp>

#include <vector>

namespace bench
{
    // Вклад узла: элемент + атрибуты, текст и CDATA — по одному / The node contribution: an element + attributes, text and CDATA — one each
    static std::size_t weight(const xercesc::DOMNode* node)
    {
        xercesc::DOMNode::NodeType type = node->getNodeType();

        if (type == xercesc::DOMNode::TEXT_NODE || type == xercesc::DOMNode::CDATA_SECTION_NODE)
            return 1;
        if (type != xercesc::DOMNode::ELEMENT_NODE)
            return 0;
        return 1 + node->getAttributes()->getLength();
    }

    // Обход по явному стеку / A walk with an explicit stack
    std::size_t XercesEngine::traverse() const
    {
        std::vector<const xercesc::DOMNode*> stack{_document};
        std::size_t sum = 0;

        while (!stack.empty())
        {
            const xercesc::DOMNode* node = stack.back();
            stack.pop_back();
            sum += weight(node);
            for (const xercesc::DOMNode* child = node->getFirstChild(); child; child = child->getNextSibling())
                stack.push_back(child);
        }
        return sum;
    }

    /*
    ** Вывод через DOMLSSerializer в буфер памяти в UTF-8 — стандартный путь
    ** Xerces (внутренние строки UTF-16 при этом перекодируются).
    **
    ** Output through DOMLSSerializer into a memory buffer in UTF-8 — the standard
    ** Xerces path (the internal UTF-16 strings are transcoded along the way).
    */
    std::optional<std::size_t> XercesEngine::serialize() const
    {
        static const XMLCh LS[] = {u'L', u'S', 0};
        static const XMLCh UTF8[] = {u'U', u'T', u'F', u'-', u'8', 0};
        auto* implementation = static_cast<xercesc::DOMImplementationLS*>(
            xercesc::DOMImplementationRegistry::getDOMImplementation(LS));
        xercesc::DOMLSSerializer* serializer = implementation->createLSSerializer();
        xercesc::DOMLSOutput* output = implementation->createLSOutput();
        xercesc::MemBufFormatTarget target;

        output->setByteStream(&target);
        output->setEncoding(UTF8);
        serializer->write(_document, output);
        std::size_t size = target.getLen();
        output->release();
        serializer->release();
        return size;
    }
}
