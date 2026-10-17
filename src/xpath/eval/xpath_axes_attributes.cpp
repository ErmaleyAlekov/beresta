#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_axes.hpp"
#include "core/processing_instruction.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xpath::detail
{
    // Атрибуты элемента; объявления xmlns в модели XPath атрибутами не считаются
    void attributeAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        if (item.attribute != nullptr || item.node == nullptr
            || item.node->getType() != NodeType::ELEMENT)
            return;
        const Element* element = static_cast<const Element*>(item.node);
        for (const Attribute& attribute : element->getAttributes())
        {
            if (!utils::isNamespaceDeclaration(attribute.name))
                output.push_back(XPathItem{nullptr, &attribute, element});
        }
    }

    // Локальное имя и URI пространства имён элемента или атрибута
    static void itemName(const XPathItem& item, std::string_view& local, std::string& uri)
    {
        if (item.attribute == nullptr)
        {
            const Element* element = static_cast<const Element*>(item.node);
            local = element->getLocalName();
            uri = element->getNamespaceURI().value_or("");
            return;
        }
        utils::QualifiedNameParts parts = utils::splitQualifiedName(item.attribute->name);
        local = parts.localName;
        if (!parts.prefix.empty())
            uri = item.owner->lookupNamespaceURI(parts.prefix).value_or("");
    }

    /*
    ** Проверка имени. Главный тип оси attribute — атрибуты, остальных —
    ** элементы. Префикс из выражения разрешается через XPathContext;
    ** имя без префикса означает «вне пространств имён» (XPath 1.0).
    */
    static bool matchesName(const Step& step, const XPathItem& item, const XPathContext& user)
    {
        bool attributeAxis = step.axis == Axis::ATTRIBUTE_AXIS;

        if (attributeAxis != (item.attribute != nullptr))
            return false;
        if (!attributeAxis && (item.node == nullptr || item.node->getType() != NodeType::ELEMENT))
            return false;
        if (step.test == TestKind::ANY_NAME)
            return true;
        std::string_view local;
        std::string uri;
        itemName(item, local, uri);
        if (step.test == TestKind::QUALIFIED_NAME && local != step.localName)
            return false;
        std::string expected;
        if (!step.prefix.empty())
        {
            auto found = user.namespaces.find(step.prefix);
            if (found == user.namespaces.end())
                failEvaluation("префикс \"" + step.prefix + "\" не связан в XPathContext");
            expected = found->second;
        }
        return uri == expected;
    }

    // Проверка узла: тип узла или имя
    bool matchesTest(const Step& step, const XPathItem& item, const XPathContext& user)
    {
        if (step.test == TestKind::ANY_NODE)
            return true;
        if (step.test != TestKind::TEXT_NODE && step.test != TestKind::COMMENT_NODE
            && step.test != TestKind::PI_NODE)
            return matchesName(step, item, user);
        if (item.attribute != nullptr || item.node == nullptr)
            return false;
        NodeType type = item.node->getType();
        if (step.test == TestKind::TEXT_NODE)
            return type == NodeType::TEXT || type == NodeType::CDATA_SECTION;
        if (step.test == TestKind::COMMENT_NODE)
            return type == NodeType::COMMENT;
        if (type != NodeType::PROCESSING_INSTRUCTION)
            return false;
        return step.target.empty()
            || static_cast<const ProcessingInstruction*>(item.node)->getTarget() == step.target;
    }
}
