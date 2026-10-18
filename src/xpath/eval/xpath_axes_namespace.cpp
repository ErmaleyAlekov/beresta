#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_axes.hpp"
#include "utils/qualified_name.hpp"

#include <set>
#include <string_view>

namespace beresta::xpath::detail
{
    /*
    ** Префикс xml связан с этим URI всегда и без объявления (Namespaces in
    ** XML), поэтому у каждого элемента есть узел пространства имён xml.
    ** Для него нет атрибута в документе — узел указывает на этот образец.
    **
    ** The xml prefix is always bound to this URI without a declaration (Namespaces in
    ** XML), so every element has an xml namespace node.
    ** There is no attribute for it in the document — the node points to this sample.
    */
    static const Attribute XML_NAMESPACE_DECLARATION{"xmlns:xml", "http://www.w3.org/XML/1998/namespace"};

    /*
    ** Ось namespace (XPath 1.0, раздел 5.4): по одному узлу на каждый
    ** префикс, действующий на элементе. Объявления собираются от элемента
    ** вверх по предкам; ближайшее объявление префикса скрывает дальние.
    ** xmlns="" отменяет пространство по умолчанию — узла для него нет.
    ** У остальных узлов (не элементов) ось namespace пуста.
    **
    ** The namespace axis (XPath 1.0, section 5.4): one node for each
    ** prefix in scope on the element. The declarations are collected from the element
    ** up through its ancestors; the nearest declaration of a prefix hides the farther ones.
    ** xmlns="" cancels the default namespace — there is no node for it.
    ** For other nodes (not elements) the namespace axis is empty.
    */
    void namespaceAxis(const XPathItem& item, std::vector<XPathItem>& output)
    {
        if (item.attribute != nullptr || item.node == nullptr || item.node->getType() != NodeType::ELEMENT)
            return;
        const Element* owner = static_cast<const Element*>(item.node);
        std::set<std::string_view> seen;
        for (const Node* node = owner; node != nullptr; node = node->getParent())
        {
            if (node->getType() != NodeType::ELEMENT)
                continue;
            for (const Attribute& attribute : static_cast<const Element*>(node)->getAttributes())
            {
                if (!utils::isNamespaceDeclaration(attribute.name))
                    continue;
                bool first = seen.insert(utils::declaredPrefix(attribute.name)).second;
                if (first && !attribute.value.empty())
                    output.push_back(XPathItem{nullptr, &attribute, owner, true});
            }
        }
        if (seen.insert("xml").second)
            output.push_back(XPathItem{nullptr, &XML_NAMESPACE_DECLARATION, owner, true});
    }
}
