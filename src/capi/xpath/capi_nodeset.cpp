#include "capi/internal/capi_support.hpp"

namespace
{
    // Элемент набора по индексу с проверкой диапазона
    const beresta::xpath::XPathItem& itemAt(beresta_nodeset nodeset, int64_t index)
    {
        const beresta::capi::NodeSetHandle& handle =
            beresta::capi::requireHandle<beresta::capi::NodeSetHandle>(nodeset, "nodeset");

        return handle.items[beresta::capi::checkedIndex(index, handle.items.size())];
    }
}

extern "C" BERESTA_API int64_t beresta_nodeset_size(beresta_nodeset nodeset)
{
    BERESTA_CAPI_TRY
        return static_cast<int64_t>(beresta::capi::requireHandle<beresta::capi::NodeSetHandle>(nodeset, "nodeset").items.size());
    BERESTA_CAPI_CATCH(-1)
}

/*
** Узел набора. В DOM «Берёсты» атрибут — свойство элемента, а не узел,
** поэтому для атрибута возвращается NULL (код 0), а его значение
** доступно через beresta_nodeset_value. Ядро хранит const-указатели;
** Си-интерфейс не различает константность, отсюда const_cast.
*/
extern "C" BERESTA_API beresta_node beresta_nodeset_node(beresta_nodeset nodeset, int64_t index)
{
    BERESTA_CAPI_TRY
        const beresta::xpath::XPathItem& item = itemAt(nodeset, index);

        if (item.attribute != nullptr)
            return nullptr;
        return const_cast<beresta::Node*>(item.node);
    BERESTA_CAPI_CATCH(nullptr)
}

// Строковое значение узла или атрибута (string-value по XPath)
extern "C" BERESTA_API char* beresta_nodeset_value(beresta_nodeset nodeset, int64_t index)
{
    BERESTA_CAPI_TRY
        return beresta::capi::duplicateString(beresta::xpath::stringValue(itemAt(nodeset, index)));
    BERESTA_CAPI_CATCH(nullptr)
}

extern "C" BERESTA_API void beresta_nodeset_free(beresta_nodeset nodeset)
{
    delete static_cast<beresta::capi::NodeSetHandle*>(nodeset);
}
