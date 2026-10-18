#include "conformance/canonical_writer.hpp"

#include "beresta.hpp"

#include <algorithm>
#include <vector>

namespace conformance
{
    namespace
    {
        // Порядок атрибутов канонической формы: по имени (побайтово = по кодам Unicode в UTF-8)
        // The attribute order of the canonical form: by name (byte-wise = by Unicode codes in UTF-8)
        bool attributeNameLess(const beresta::Attribute* left, const beresta::Attribute* right)
        {
            return left->name < right->name;
        }
    }

    /*
    ** На верхнем уровне в канонической форме остаются только корневой
    ** элемент и инструкции обработки: DOCTYPE и комментарии опускаются.
    **
    ** At the top level only the root element and processing instructions stay
    ** in the canonical form: the DOCTYPE and comments are omitted.
    */
    std::string writeCanonical(const beresta::Document& document)
    {
        std::string output;

        for (std::size_t index = 0; index < document.getChildCount(); ++index)
        {
            const beresta::Node* child = document.getChild(index);
            beresta::NodeType type = child->getType();

            if (type == beresta::NodeType::ELEMENT || type == beresta::NodeType::PROCESSING_INSTRUCTION)
                detail::writeCanonicalNode(*child, output);
        }
        return output;
    }

    namespace detail
    {
        /*
        ** Рекурсия по дереву допустима: глубина документов набора W3C
        ** невелика, а парсер в любом случае ограничивает её maxDepth.
        **
        ** Recursion over the tree is acceptable here: the documents of the W3C suite
        ** are not deep, and the parser limits the depth with maxDepth anyway.
        */
        void writeCanonicalNode(const beresta::Node& node, std::string& output)
        {
            beresta::NodeType type = node.getType();

            if (type == beresta::NodeType::TEXT || type == beresta::NodeType::CDATA_SECTION)
                appendCanonicalEscaped(static_cast<const beresta::CharacterNode&>(node).getValue(), output);
            if (type == beresta::NodeType::PROCESSING_INSTRUCTION)
            {
                const auto& instruction = static_cast<const beresta::ProcessingInstruction&>(node);

                // Ровно один пробел между целью и данными — даже при пустых данных
                // Exactly one space between the target and the data — even with empty data
                output += "<?" + instruction.getTarget() + " " + instruction.getData() + "?>";
            }
            if (type != beresta::NodeType::ELEMENT)
                return;
            const auto& element = static_cast<const beresta::Element&>(node);

            writeCanonicalStartTag(element, output);
            for (std::size_t index = 0; index < element.getChildCount(); ++index)
                writeCanonicalNode(*element.getChild(index), output);
            output += "</" + element.getName() + ">";
        }

        // Атрибуты (включая объявления xmlns) сортируются по полному имени
        // Attributes (including xmlns declarations) are sorted by full name
        void writeCanonicalStartTag(const beresta::Element& element, std::string& output)
        {
            std::vector<const beresta::Attribute*> sorted;

            for (const beresta::Attribute& attribute : element.getAttributes())
                sorted.push_back(&attribute);
            std::sort(sorted.begin(), sorted.end(), attributeNameLess);
            output += "<" + element.getName();
            for (const beresta::Attribute* attribute : sorted)
            {
                output += " " + attribute->name + "=\"";
                appendCanonicalEscaped(attribute->value, output);
                output += "\"";
            }
            output += ">";
        }
    }
}
