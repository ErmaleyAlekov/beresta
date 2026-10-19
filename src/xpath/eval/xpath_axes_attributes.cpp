#include "xpath/internal/xpath_eval.hpp"
#include "i18n/i18n.hpp"
#include "xpath/internal/xpath_axes.hpp"
#include "core/processing_instruction.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xpath::detail
{
    // Атрибуты элемента; объявления xmlns в модели XPath атрибутами не считаются
    // The attributes of an element; xmlns declarations are not attributes in the XPath model
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

    /*
    ** Совпадает ли локальное имя элемента или атрибута с проверкой шага; при
    ** совпадении в uri кладётся URI его пространства имён. Сначала дешёвое
    ** сравнение имени и только потом поиск URI по объявлениям xmlns предков:
    ** у //item большинство кандидатов отсеивается уже на имени.
    **
    ** Whether the local name of an element or an attribute matches the step test;
    ** on a match its namespace URI is put into uri. First the cheap name comparison
    ** and only then the URI lookup through the xmlns declarations of the ancestors:
    ** for //item most candidates are rejected already by the name.
    */
    static bool itemNameMatches(const Step& step, const XPathItem& item, std::string& uri)
    {
        bool byName = step.test == TestKind::QUALIFIED_NAME;

        if (item.attribute == nullptr)
        {
            const Element* element = static_cast<const Element*>(item.node);
            if (byName && element->getLocalName() != step.localName)
                return false;
            uri = element->getNamespaceURI().value_or("");
            return true;
        }
        utils::QualifiedNameParts parts = utils::splitQualifiedName(item.attribute->name);
        if (byName && parts.localName != step.localName)
            return false;
        if (!parts.prefix.empty())
            uri = item.owner->lookupNamespaceURI(parts.prefix).value_or("");
        return true;
    }

    /*
    ** Проверка имени на оси namespace. Главный тип узла этой оси — узел
    ** пространства имён: его имя — префикс, а URI имени пуст. Поэтому "*"
    ** выбирает все такие узлы, "p" — узел префикса p, а "p:*" и "p:x" —
    ** ни одного (у имени узла пространства имён нет URI).
    **
    ** The name test on the namespace axis. The principal node type of this axis is the
    ** namespace node: its name is the prefix, and the URI of the name is empty. So "*"
    ** selects all such nodes, "p" selects the node of prefix p, and "p:*" and "p:x" select
    ** none (the name of a namespace node has no URI).
    */
    static bool matchesNamespaceNode(const Step& step, const XPathItem& item)
    {
        if (!item.isNamespace)
            return false;
        if (step.test == TestKind::ANY_NAME)
            return true;
        return step.test == TestKind::QUALIFIED_NAME && step.prefix.empty()
            && utils::declaredPrefix(item.attribute->name) == step.localName;
    }

    /*
    ** Проверка имени. Имя проверяется только у узлов главного типа оси:
    ** атрибутов на оси attribute, узлов пространств имён на оси namespace,
    ** элементов на остальных осях. Имя без префикса — вне пространств имён;
    ** лишь в утверждениях XSD имя элемента без префикса берёт пространство
    ** xpathDefaultNamespace (оно лежит в namespaces под пустым префиксом).
    **
    ** The name test. The name is checked only for nodes of the principal type of the axis:
    ** attributes on the attribute axis, namespace nodes on the namespace axis,
    ** elements on the other axes. A name without a prefix is in no namespace;
    ** only in XSD assertions an element name without a prefix takes the
    ** xpathDefaultNamespace namespace (it lies in namespaces under the empty prefix).
    */
    static bool matchesName(const Step& step, const XPathItem& item, const EvalContext& context)
    {
        bool attributeAxis = step.axis == Axis::ATTRIBUTE_AXIS;

        if (step.axis == Axis::NAMESPACE_AXIS || item.isNamespace)
            return step.axis == Axis::NAMESPACE_AXIS && matchesNamespaceNode(step, item);
        if (attributeAxis != (item.attribute != nullptr))
            return false;
        if (!attributeAxis && (item.node == nullptr || item.node->getType() != NodeType::ELEMENT))
            return false;
        if (step.test == TestKind::ANY_NAME)
            return true;
        std::string uri;
        if (!itemNameMatches(step, item, uri))
            return false;
        std::string expected;
        if (step.prefix.empty() && (context.typed == nullptr || attributeAxis))
            return uri.empty();
        auto found = context.user.namespaces.find(step.prefix);
        if (found != context.user.namespaces.end())
            expected = found->second;
        else if (!step.prefix.empty())
            failEvaluation(i18n::format(i18n::Message::XPATH_UNBOUND_PREFIX, {step.prefix}));
        return uri == expected;
    }

    // Проверка узла: тип узла или имя
    // The node test: the node type or the name
    bool matchesTest(const Step& step, const XPathItem& item, const EvalContext& context)
    {
        if (step.test == TestKind::ANY_NODE)
            return true;
        if (step.test != TestKind::TEXT_NODE && step.test != TestKind::COMMENT_NODE
            && step.test != TestKind::PI_NODE)
            return matchesName(step, item, context);
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
