#include "xpath/internal/xpath_parser.hpp"
#include "i18n/i18n.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Имя оси и её значение
    // The axis name and its value
    struct AxisEntry
    {
        const char* name;
        Axis axis;
    };

    // Все 13 осей XPath 1.0 (узлы namespace строятся по объявлениям xmlns)
    // All 13 XPath 1.0 axes (namespace nodes are built from the xmlns declarations)
    static const AxisEntry AXES[] = {
        {"ancestor", Axis::ANCESTOR}, {"ancestor-or-self", Axis::ANCESTOR_OR_SELF},
        {"attribute", Axis::ATTRIBUTE_AXIS}, {"child", Axis::CHILD},
        {"descendant", Axis::DESCENDANT}, {"descendant-or-self", Axis::DESCENDANT_OR_SELF},
        {"following", Axis::FOLLOWING}, {"following-sibling", Axis::FOLLOWING_SIBLING},
        {"parent", Axis::PARENT}, {"preceding", Axis::PRECEDING},
        {"preceding-sibling", Axis::PRECEDING_SIBLING}, {"self", Axis::SELF},
        {"namespace", Axis::NAMESPACE_AXIS}
    };

    // Поиск оси по имени
    // Looks up an axis by name
    static bool findAxis(const std::string& name, Axis& axis) noexcept
    {
        for (const AxisEntry& entry : AXES)
        {
            if (name == entry.name)
            {
                axis = entry.axis;
                return true;
            }
        }
        return false;
    }

    // Путь начинается с '/', '//', '.', '..', '@', оси, проверки имени или типа узла
    // A path starts with '/', '//', '.', '..', '@', an axis, a name test or a node type
    bool XPathParser::startsLocationPath() const noexcept
    {
        switch (peek().type)
        {
            case TokenType::SLASH: case TokenType::DOUBLE_SLASH: case TokenType::DOT:
            case TokenType::DOUBLE_DOT: case TokenType::AT_SIGN: case TokenType::AXIS_NAME:
            case TokenType::NAME_TEST: case TokenType::NODE_TYPE:
                return true;
            default:
                return false;
        }
    }

    /*
    ** Абсолютный путь "/..." или "//...", либо относительный.
    ** Одиночный "/" — корень документа (шагов нет).
    **
    ** An absolute path "/..." or "//...", or a relative one.
    ** A single "/" is the document root (there are no steps).
    */
    ExprPtr XPathParser::parseLocationPath()
    {
        std::vector<Step> steps;
        bool absolute = peek().type == TokenType::SLASH || peek().type == TokenType::DOUBLE_SLASH;

        if (accept(TokenType::DOUBLE_SLASH))
        {
            steps.push_back(descendantStep());
            parseRelativePath(steps);
        }
        else if (accept(TokenType::SLASH))
        {
            if (startsLocationPath() && peek().type != TokenType::SLASH)
                parseRelativePath(steps);
        }
        else
            parseRelativePath(steps);
        return std::make_unique<PathExpr>(nullptr, absolute, std::move(steps));
    }

    // Шаг ('/' | '//') шаг ...
    // A step ('/' | '//') step ...
    void XPathParser::parseRelativePath(std::vector<Step>& steps)
    {
        steps.push_back(parseStep());
        while (peek().type == TokenType::SLASH || peek().type == TokenType::DOUBLE_SLASH)
        {
            if (take().type == TokenType::DOUBLE_SLASH)
                steps.push_back(descendantStep());
            steps.push_back(parseStep());
        }
    }

    /*
    ** Шаг: сокращения '.' (self::node()) и '..' (parent::node()),
    ** либо [ось:: | @] проверка [предикаты]. По умолчанию ось — child.
    **
    ** A step: the abbreviations '.' (self::node()) and '..' (parent::node()),
    ** or [axis:: | @] test [predicates]. The default axis is child.
    */
    Step XPathParser::parseStep()
    {
        Step step;

        if (accept(TokenType::DOT) || accept(TokenType::DOUBLE_DOT))
        {
            if (_tokens[_index - 1].type == TokenType::DOUBLE_DOT)
                step.axis = Axis::PARENT;
            else
                step.axis = Axis::SELF;
            return step;
        }
        if (accept(TokenType::AT_SIGN))
            step.axis = Axis::ATTRIBUTE_AXIS;
        else if (peek().type == TokenType::AXIS_NAME)
        {
            std::string name = take().text;
            if (!findAxis(name, step.axis))
                fail(i18n::format(i18n::Message::XPATH_UNKNOWN_AXIS, {name}));
            expect(TokenType::DOUBLE_COLON, "'::'");
        }
        parseNodeTest(step);
        parsePredicates(step.predicates);
        return step;
    }
}
