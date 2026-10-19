#include "xpath/internal/xpath_typed.hpp"
#include "xpath/internal/xpath_eval.hpp"
#include "i18n/i18n.hpp"

#include <string>

namespace beresta::xpath::detail
{
    /*
    ** xs:boolean: строка — только "true", "false", "1", "0" (с пробелами по
    ** краям), число — «не ноль», дата — ошибка.
    **
    ** xs:boolean: a string — only "true", "false", "1", "0" (with spaces
    ** around), a number — "not zero", a date — an error.
    */
    static XPathValue castBooleanAtom(const XPathValue& atom)
    {
        if (atom.getType() == ValueType::BOOLEAN)
            return atom;
        if (isDateValue(atom))
            failCast(atom.toString(), "boolean");
        if (atom.getType() != ValueType::STRING)
            return XPathValue::fromBoolean(atom.toBoolean());
        std::string text = atom.toString();
        std::size_t first = text.find_first_not_of(" \t\r\n");
        std::size_t last = text.find_last_not_of(" \t\r\n");
        std::string trimmed;
        if (first != std::string::npos)
            trimmed = text.substr(first, last - first + 1);
        if (trimmed == "true" || trimmed == "1")
            return XPathValue::fromBoolean(true);
        if (trimmed != "false" && trimmed != "0")
            failCast(text, "boolean");
        return XPathValue::fromBoolean(false);
    }

    /*
    ** xs:decimal / xs:integer: точное число из атома; дробный double —
    ** через его запись XPath (без экспоненты); логическое — 1 или 0.
    ** xs:integer из строки требует целую запись, из числа — отбрасывает дробь.
    **
    ** xs:decimal / xs:integer: an exact number from the atom; a fractional double —
    ** through its XPath notation (without an exponent); a boolean — 1 or 0.
    ** xs:integer from a string requires an integer notation, from a number — drops the fraction.
    */
    static XPathValue castDecimalAtom(bool integer, const XPathValue& atom)
    {
        utils::Decimal result;
        std::string text = atom.toString();
        const char* name = "decimal";
        bool converted = exactDecimal(atom, result);

        if (integer)
            name = "integer";
        if (!converted && atom.getType() == ValueType::NUMBER)
            converted = utils::readDecimal(numberToString(atom.toNumber()), result);
        if (!converted && atom.getType() == ValueType::BOOLEAN)
        {
            result = utils::decimalFromInteger(static_cast<long long>(atom.toBoolean()));
            converted = true;
        }
        if (!converted || (integer && atom.getType() == ValueType::STRING && text.find('.') != std::string::npos))
            failCast(text, name);
        if (integer)
            result = utils::truncateDecimal(result);
        return XPathValue::fromDecimal(result);
    }

    // xs:date / xs:dateTime: дата того же типа — как есть, строка — разбором по лексике XSD
    // xs:date / xs:dateTime: a date of the same type — as is, a string — by parsing with the XSD lexis
    static XPathValue castDateAtom(AtomicType type, const XPathValue& atom, const EvalContext& context)
    {
        ValueType kind = ValueType::DATE_TIME;
        const char* name = "dateTime";
        XPathValue result = XPathValue::fromBoolean(false);

        if (type == AtomicType::DATE)
        {
            kind = ValueType::DATE;
            name = "date";
        }
        if (atom.getType() == kind)
            return atom;
        if (atom.getType() == ValueType::STRING && context.typed->castDate(kind, atom.toString(), result))
            return result;
        failCast(atom.toString(), name);
    }

    /*
    ** Конструктор xs:тип(аргумент) — приведение одного атома. Пустой
    ** аргумент даёт пустую последовательность (пустое множество узлов).
    **
    ** The constructor xs:type(argument) is a cast of one atom. An empty
    ** argument gives an empty sequence (an empty node set).
    */
    XPathValue castAtomic(AtomicType type, const XPathValue& value, const EvalContext& context)
    {
        XPathValue atom = XPathValue::fromBoolean(false);

        if (!singleAtom(value, context, atom))
            return XPathValue::fromNodeSet(std::vector<XPathItem>());
        if (type == AtomicType::STRING)
            return XPathValue::fromString(atom.toString());
        if (type == AtomicType::BOOLEAN)
            return castBooleanAtom(atom);
        if (type == AtomicType::DATE || type == AtomicType::DATE_TIME)
            return castDateAtom(type, atom, context);
        if (type != AtomicType::DOUBLE)
            return castDecimalAtom(type == AtomicType::INTEGER, atom);
        if (isDateValue(atom))
            failCast(atom.toString(), "double");
        return XPathValue::fromNumber(numericDouble(atom));
    }

    // Ошибка приведения FORG0001 с текстом значения и именем типа
    // The FORG0001 cast error with the value text and the type name
    void failCast(const std::string& value, const char* typeName)
    {
        failEvaluation(i18n::format(i18n::Message::XPATH_CAST_FAILED, {value, typeName}));
    }
}
