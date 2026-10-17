#include "xpath/internal/xpath_parser.hpp"

#include <utility>

namespace beresta::xpath::detail
{
    // Имя оси и её значение
    struct AxisEntry
    {
        const char* name;
        Axis axis;
    };

    // Все оси XPath 1.0, кроме namespace (у DOM «Берёсты» нет узлов пространств имён)
    static const AxisEntry AXES[] = {
        {"ancestor", Axis::ANCESTOR}, {"ancestor-or-self", Axis::ANCESTOR_OR_SELF},
        {"attribute", Axis::ATTRIBUTE_AXIS}, {"child", Axis::CHILD},
        {"descendant", Axis::DESCENDANT}, {"descendant-or-self", Axis::DESCENDANT_OR_SELF},
        {"following", Axis::FOLLOWING}, {"following-sibling", Axis::FOLLOWING_SIBLING},
        {"parent", Axis::PARENT}, {"preceding", Axis::PRECEDING},
        {"preceding-sibling", Axis::PRECEDING_SIBLING}, {"self", Axis::SELF}
    };

    // Поиск оси по имени
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
                fail("неподдерживаемая ось \"" + name + "\"");
            expect(TokenType::DOUBLE_COLON, "'::'");
        }
        parseNodeTest(step);
        parsePredicates(step.predicates);
        return step;
    }
}
