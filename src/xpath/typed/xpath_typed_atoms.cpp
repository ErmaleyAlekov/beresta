#include "xpath/internal/xpath_typed.hpp"
#include "xpath/internal/xpath_eval.hpp"
#include "i18n/i18n.hpp"

#include <cmath>
#include <string>

namespace beresta::xpath::detail
{
    /*
    ** Атомизация (XPath 2.0, раздел 2.4.2): каждый узел множества заменяется
    ** своим типизированным значением; атомарное значение остаётся собой.
    **
    ** Atomization (XPath 2.0, section 2.4.2): each node of the set is replaced
    ** with its typed value; an atomic value stays itself.
    */
    std::vector<XPathValue> atomizeValue(const XPathValue& value, const EvalContext& context)
    {
        std::vector<XPathValue> atoms;

        if (value.getType() != ValueType::NODE_SET)
        {
            atoms.push_back(value);
            return atoms;
        }
        for (const XPathItem& item : value.getNodes())
            atoms.push_back(context.typed->atomize(item));
        return atoms;
    }

    /*
    ** Операнд арифметики, сравнения значений или конструктора — не больше
    ** одного атома. Пустое множество — «пустая последовательность» (false),
    ** несколько узлов — ошибка типа XPath 2.0 (XPTY0004): молча брать первый
    ** узел, как XPath 1.0, в банковской сумме опасно.
    **
    ** An operand of arithmetic, a value comparison or a constructor is at most
    ** one atom. An empty set is "an empty sequence" (false),
    ** several nodes are the XPath 2.0 type error (XPTY0004): silently taking the first
    ** node, as XPath 1.0 does, is dangerous in a banking sum.
    */
    bool singleAtom(const XPathValue& value, const EvalContext& context, XPathValue& atom)
    {
        if (value.getType() != ValueType::NODE_SET)
        {
            atom = value;
            return true;
        }
        std::size_t count = value.getNodes().size();
        if (count == 0)
            return false;
        if (count > 1)
            failEvaluation(i18n::format(i18n::Message::XPATH_SEVERAL_ITEMS, {std::to_string(count)}));
        atom = context.typed->atomize(value.getNodes().front());
        return true;
    }

    /*
    ** Точное число: xs:decimal как есть; строка (нетипизированный узел) — если
    ** это запись decimal (так "0.10" из нетипизированного элемента тоже
    ** складывается точно); double — только целое (count(), position()).
    **
    ** An exact number: xs:decimal as is; a string (an untyped node) — if
    ** it is a decimal notation (so "0.10" from an untyped element is also
    ** added exactly); a double — only an integer one (count(), position()).
    */
    bool exactDecimal(const XPathValue& atom, utils::Decimal& result)
    {
        if (atom.getType() == ValueType::DECIMAL)
        {
            result = atom.getDecimal();
            return true;
        }
        if (atom.getType() == ValueType::STRING)
            return utils::readDecimal(atom.toString(), result);
        if (atom.getType() == ValueType::NUMBER)
            return utils::decimalFromDouble(atom.toNumber(), result);
        return false;
    }

    // Атом как double: строка обязана быть числом (иначе — ошибка FORG0001 вместо тихого NaN)
    // An atom as a double: a string must be a number (otherwise — the FORG0001 error instead of a silent NaN)
    double numericDouble(const XPathValue& atom)
    {
        if (atom.getType() != ValueType::STRING)
            return atom.toNumber();
        double number = stringToNumber(atom.toString());
        if (std::isnan(number))
            failCast(atom.toString(), "double");
        return number;
    }

    // DATE или DATE_TIME
    // DATE or DATE_TIME
    bool isDateValue(const XPathValue& value) noexcept
    {
        return value.getType() == ValueType::DATE || value.getType() == ValueType::DATE_TIME;
    }
}
