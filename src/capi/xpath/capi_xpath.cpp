#include "capi/internal/capi_support.hpp"

/*
** Скомпилированное выражение вместе с контекстом (префиксы и переменные).
** Компиляция дорогая, вычисление дешёвое: обёртки компилируют выражение
** один раз и вычисляют его для множества узлов и документов.
** Пока контекст меняется (bind / set), выражение нельзя вычислять
** из других потоков; само вычисление контекст не изменяет.
*/
extern "C" BERESTA_API int beresta_xpath_compile(const char* expression, beresta_xpath* out)
{
    BERESTA_CAPI_TRY
        std::string_view source = beresta::capi::requireText(expression, "expression");

        if (out == nullptr)
            throw beresta::capi::InvalidArgumentException("out: передан NULL");
        std::unique_ptr<beresta::capi::XPathHandle> handle(new beresta::capi::XPathHandle{
            beresta::xpath::XPathExpression(source), beresta::xpath::XPathContext()});
        *out = handle.release();
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

extern "C" BERESTA_API void beresta_xpath_free(beresta_xpath xpath)
{
    delete static_cast<beresta::capi::XPathHandle*>(xpath);
}

// Префикс для имён p:name в выражении (повторная привязка заменяет URI)
extern "C" BERESTA_API int beresta_xpath_bind_namespace(beresta_xpath xpath, const char* prefix,
    const char* uri)
{
    BERESTA_CAPI_TRY
        beresta::capi::XPathHandle& handle = beresta::capi::requireHandle<beresta::capi::XPathHandle>(xpath, "xpath");
        std::string key(beresta::capi::requireText(prefix, "prefix"));

        handle.context.namespaces.insert_or_assign(std::move(key),
            std::string(beresta::capi::requireText(uri, "uri")));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

extern "C" BERESTA_API int beresta_xpath_set_string(beresta_xpath xpath, const char* name, const char* value)
{
    BERESTA_CAPI_TRY
        beresta::capi::XPathHandle& handle = beresta::capi::requireHandle<beresta::capi::XPathHandle>(xpath, "xpath");
        std::string key(beresta::capi::requireText(name, "name"));
        std::string text(beresta::capi::requireText(value, "value"));

        handle.context.variables.insert_or_assign(std::move(key), beresta::xpath::XPathValue::fromString(std::move(text)));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

extern "C" BERESTA_API int beresta_xpath_set_number(beresta_xpath xpath, const char* name, double value)
{
    BERESTA_CAPI_TRY
        beresta::capi::XPathHandle& handle = beresta::capi::requireHandle<beresta::capi::XPathHandle>(xpath, "xpath");
        std::string key(beresta::capi::requireText(name, "name"));

        handle.context.variables.insert_or_assign(std::move(key), beresta::xpath::XPathValue::fromNumber(value));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}
