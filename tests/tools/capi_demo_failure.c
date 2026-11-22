#include "capi/beresta_c.h"

#include <stdlib.h>
#include <string.h>

/* Подменяем имя только для выбранного вызова; библиотека возвращает обычную ошибку аргумента. */
/* Override the name only for the selected call so the library returns its normal argument error. */
static const char* simulated_name(const char* operation, const char* name)
{
    const char* failure = getenv("BERESTA_DEMO_FAILURE");

    if (failure && strcmp(failure, operation) == 0)
        return NULL;
    return name;
}

/* Точка подмены для привязки XPath / Injection point for XPath binding */
static int bind_string(beresta_xpath xpath, const char* name, const char* value)
{
    return beresta_xpath_set_string(xpath, simulated_name("xpath", name), value);
}

/* Точка подмены для атрибута / Injection point for setting an attribute */
static int set_attribute(beresta_node node, const char* name, const char* value)
{
    return beresta_element_set_attribute(node, simulated_name("attribute", name), value);
}

#define beresta_xpath_set_string bind_string
#define beresta_element_set_attribute set_attribute
#include "../../examples/capi/capi_demo.c"
