#include "xpath/xpath.hpp"
#include "i18n/i18n.hpp"
#include "xpath/internal/xpath_eval.hpp"
#include "xpath/internal/xpath_parser.hpp"

namespace beresta::xpath
{
    // Компиляция: лексемы -> дерево разбора; дерево неизменяемо и разделяется копиями
    // Compilation: tokens -> a parse tree; the tree is immutable and shared by copies
    XPathExpression::XPathExpression(std::string_view expression)
        : _source(expression), _root()
    {
        detail::XPathLexer lexer(expression);
        detail::XPathParser parser(lexer.tokenize());

        _root = std::shared_ptr<const detail::Expr>(parser.parse());
    }

    // Контекст — указанный узел в позиции 1 из 1
    // The context is the given node at position 1 of 1
    XPathValue XPathExpression::evaluate(const Node& context, const XPathContext& variables) const
    {
        detail::EvalContext evalContext{XPathItem{&context, nullptr, nullptr}, 1, 1, variables};

        return _root->evaluate(evalContext);
    }

    // Элементы из результата-множества (атрибуты и текст пропускаются)
    // The elements of the result set (attributes and text are skipped)
    std::vector<const Element*> XPathExpression::selectElements(const Node& context,
        const XPathContext& variables) const
    {
        XPathValue value = evaluate(context, variables);
        std::vector<const Element*> elements;

        if (value.getType() != ValueType::NODE_SET)
            detail::failEvaluation(i18n::format(i18n::Message::XPATH_NOT_NODESET, {_source}));
        for (const XPathItem& item : value.getNodes())
        {
            if (item.attribute == nullptr && item.node->getType() == NodeType::ELEMENT)
                elements.push_back(static_cast<const Element*>(item.node));
        }
        return elements;
    }

    /*
    ** Изменяемая версия: узлы найдены в поддереве изменяемого узла,
    ** поэтому снятие константности безопасно.
    **
    ** The mutable version: the nodes are found in the subtree of a mutable node,
    ** so removing the constness is safe.
    */
    std::vector<Element*> XPathExpression::selectElements(Node& context,
        const XPathContext& variables) const
    {
        const Node& constant = context;
        std::vector<Element*> elements;

        for (const Element* element : selectElements(constant, variables))
            elements.push_back(const_cast<Element*>(element));
        return elements;
    }

    // Исходный текст выражения
    // The source text of the expression
    const std::string& XPathExpression::getSource() const noexcept
    {
        return _source;
    }
}
