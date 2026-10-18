#include "capi/internal/capi_support.hpp"

/*
** Атрибуты по имени. Отсутствие атрибута — не ошибка: get возвращает
** NULL с кодом 0, has / remove возвращают 0. Так обёртки легко
** превращают результат в null / None / Optional без try/catch.
**
** Attributes by name. A missing attribute is not an error: get returns
** NULL with code 0, has / remove return 0. So the wrappers easily turn
** the result into null / None / Optional without try/catch.
*/

extern "C" BERESTA_API char* beresta_element_get_attribute(beresta_node element, const char* name)
{
    BERESTA_CAPI_TRY
        const beresta::Element& self = beresta::capi::toElement(element);
        std::optional<std::string> value = self.getAttribute(beresta::capi::requireText(name, "name"));

        if (!value)
            return nullptr;
        return beresta::capi::duplicateString(*value);
    BERESTA_CAPI_CATCH(nullptr)
}

// Установка или замена значения; имя проверяется ядром (INVALID_NAME)
// Sets or replaces a value; the core checks the name (INVALID_NAME)
extern "C" BERESTA_API int beresta_element_set_attribute(beresta_node element, const char* name,
    const char* value)
{
    BERESTA_CAPI_TRY
        beresta::Element& self = beresta::capi::toElement(element);
        std::string text(beresta::capi::requireText(value, "value"));

        self.setAttribute(beresta::capi::requireText(name, "name"), std::move(text));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

extern "C" BERESTA_API int beresta_element_has_attribute(beresta_node element, const char* name)
{
    BERESTA_CAPI_TRY
        const beresta::Element& self = beresta::capi::toElement(element);

        if (self.hasAttribute(beresta::capi::requireText(name, "name")))
            return 1;
        return 0;
    BERESTA_CAPI_CATCH(-1)
}

extern "C" BERESTA_API int beresta_element_remove_attribute(beresta_node element, const char* name)
{
    BERESTA_CAPI_TRY
        beresta::Element& self = beresta::capi::toElement(element);

        if (self.removeAttribute(beresta::capi::requireText(name, "name")))
            return 1;
        return 0;
    BERESTA_CAPI_CATCH(-1)
}

extern "C" BERESTA_API int64_t beresta_element_attribute_count(beresta_node element)
{
    BERESTA_CAPI_TRY
        return static_cast<int64_t>(beresta::capi::toElement(element).getAttributes().size());
    BERESTA_CAPI_CATCH(-1)
}
