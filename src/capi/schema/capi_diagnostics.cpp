#include "capi/internal/capi_support.hpp"

namespace
{
    // Диагностика по индексу с проверкой диапазона
    const beresta::Diagnostic& diagnosticAt(beresta_diagnostics diagnostics, int64_t index)
    {
        const beresta::capi::DiagnosticsHandle& handle =
            beresta::capi::requireHandle<beresta::capi::DiagnosticsHandle>(diagnostics, "diagnostics");

        return handle.items[beresta::capi::checkedIndex(index, handle.items.size())];
    }
}

extern "C" BERESTA_API int64_t beresta_diagnostics_count(beresta_diagnostics diagnostics)
{
    BERESTA_CAPI_TRY
        return static_cast<int64_t>(beresta::capi::requireHandle<beresta::capi::DiagnosticsHandle>(
            diagnostics, "diagnostics").items.size());
    BERESTA_CAPI_CATCH(-1)
}

// Текст нарушения начинается с пути к элементу, например "/catalog/book[2]: ..."
extern "C" BERESTA_API char* beresta_diagnostics_message(beresta_diagnostics diagnostics, int64_t index)
{
    BERESTA_CAPI_TRY
        return beresta::capi::duplicateString(diagnosticAt(diagnostics, index).message);
    BERESTA_CAPI_CATCH(nullptr)
}

// Строка в исходнике; 0 — документ построен в памяти, а не разобран
extern "C" BERESTA_API int64_t beresta_diagnostics_line(beresta_diagnostics diagnostics, int64_t index)
{
    BERESTA_CAPI_TRY
        return static_cast<int64_t>(diagnosticAt(diagnostics, index).position.line);
    BERESTA_CAPI_CATCH(-1)
}

extern "C" BERESTA_API int64_t beresta_diagnostics_column(beresta_diagnostics diagnostics, int64_t index)
{
    BERESTA_CAPI_TRY
        return static_cast<int64_t>(diagnosticAt(diagnostics, index).position.column);
    BERESTA_CAPI_CATCH(-1)
}
