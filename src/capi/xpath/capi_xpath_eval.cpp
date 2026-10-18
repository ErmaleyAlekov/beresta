#include "capi/internal/capi_support.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"

namespace
{
    // Вычисление выражения относительно узла с контекстом дескриптора
    // Evaluates the expression relative to a node with the context of the handle
    beresta::xpath::XPathValue evaluate(beresta_xpath xpath, beresta_node context)
    {
        const beresta::capi::XPathHandle& handle =
            beresta::capi::requireHandle<beresta::capi::XPathHandle>(xpath, "xpath");

        return handle.expression.evaluate(beresta::capi::toNode(context), handle.context);
    }
}

/*
** Выборка узлов. Результат копируется в NodeSetHandle: элементы
** XPathItem — лёгкие указатели на узлы документа, поэтому набор
** действителен, пока документ не изменён и не освобождён.
**
** Selecting nodes. The result is copied into a NodeSetHandle: XPathItem
** items are lightweight pointers to the document nodes, so the set is
** valid while the document is not changed or freed.
*/
extern "C" BERESTA_API int beresta_xpath_select(beresta_xpath xpath, beresta_node context, beresta_nodeset* out)
{
    BERESTA_CAPI_TRY
        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::ARGUMENT_NULL, {"out"}));
        beresta::xpath::XPathValue value = evaluate(xpath, context);

        if (value.getType() != beresta::xpath::ValueType::NODE_SET)
            throw beresta::DomException(beresta::DomErrorCode::XPATH_ERROR,
                beresta::i18n::format(beresta::i18n::Message::XPATH_RESULT_NOT_NODESET));
        *out = new beresta::capi::NodeSetHandle{value.getNodes()};
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

// Результат как строка по правилам функции string()
// The result as a string by the rules of the string() function
extern "C" BERESTA_API char* beresta_xpath_string(beresta_xpath xpath, beresta_node context)
{
    BERESTA_CAPI_TRY
        return beresta::capi::duplicateString(evaluate(xpath, context).toString());
    BERESTA_CAPI_CATCH(nullptr)
}

// Результат как число по правилам функции number() (NaN — не ошибка)
// The result as a number by the rules of the number() function (NaN is not an error)
extern "C" BERESTA_API int beresta_xpath_number(beresta_xpath xpath, beresta_node context, double* out)
{
    BERESTA_CAPI_TRY
        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException(beresta::i18n::format(beresta::i18n::Message::ARGUMENT_NULL, {"out"}));
        *out = evaluate(xpath, context).toNumber();
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

// Результат как логическое значение по правилам функции boolean()
// The result as a boolean by the rules of the boolean() function
extern "C" BERESTA_API int beresta_xpath_boolean(beresta_xpath xpath, beresta_node context)
{
    BERESTA_CAPI_TRY
        if (evaluate(xpath, context).toBoolean())
            return 1;
        return 0;
    BERESTA_CAPI_CATCH(-1)
}
