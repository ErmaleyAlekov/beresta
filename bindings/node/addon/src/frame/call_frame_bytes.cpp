#include "addon/call_frame.hpp"

#include <cstdint>

namespace beresta::node
{
    // Тип Uint8Array в перечислении napi_typedarray_type / The Uint8Array type in the napi_typedarray_type enumeration
    static constexpr int UINT8_ARRAY = 1;

    /*
    ** Байты документа: Buffer или Uint8Array (Buffer — его наследник).
    ** Данные НЕ копируются: Си-функция читает память массива напрямую,
    ** а сборщик мусора не тронет массив, пока идёт синхронный вызов.
    ** Кодировку (UTF-16, windows-1251...) определяет само ядро.
    **
    ** The document bytes: a Buffer or a Uint8Array (Buffer is its subclass).
    ** The data is NOT copied: the C function reads the array memory directly,
    ** and the garbage collector does not touch the array during a synchronous
    ** call. The core itself detects the encoding (UTF-16, windows-1251...).
    */
    bool CallFrame::bytes(std::size_t index, ByteView& out) noexcept
    {
        bool typed = false;
        int kind = 0;
        std::size_t length = 0;
        void* data = nullptr;

        out = ByteView{nullptr, 0};
        if (index >= _count || napi().isTypedArray(_env, _arguments[index], &typed) != NAPI_OK || !typed)
            return reject(index, "Uint8Array");
        if (napi().getTypedArrayInfo(_env, _arguments[index], &kind, &length, &data, nullptr, nullptr) != NAPI_OK)
            return reject(index, "Uint8Array");
        if (kind != UINT8_ARRAY)
            return reject(index, "Uint8Array");
        out = ByteView{data, static_cast<std::int64_t>(length)};
        return true;
    }
}
