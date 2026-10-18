#include "addon/call_frame.hpp"

#include <cstdlib>

namespace beresta::node
{
    // Пустая строка-аргумент: указатель NULL / An empty string argument: a NULL pointer
    ArgString::ArgString() noexcept
        : _local{}, _heap(nullptr), _pointer(nullptr)
    {
    }

    // Буфер malloc освобождается всегда, даже при раннем выходе из переходника
    // The malloc buffer is always freed, even on an early exit from the adapter
    ArgString::~ArgString()
    {
        std::free(_heap);
    }

    /*
    ** Двухшаговое чтение Node-API: сначала длина строки в байтах UTF-8,
    ** затем копия в буфер подходящего размера (+1 байт на завершающий ноль).
    **
    ** The two-step Node-API read: first the string length in UTF-8 bytes,
    ** then a copy into a buffer of a suitable size (+1 byte for the terminating zero).
    */
    bool ArgString::load(napi_env env, napi_value value) noexcept
    {
        std::size_t length = 0;
        char* buffer = _local;

        if (napi().getStringUtf8(env, value, nullptr, 0, &length) != NAPI_OK)
            return false;
        if (length + 1 > LOCAL_SIZE)
        {
            _heap = static_cast<char*>(std::malloc(length + 1));
            if (_heap == nullptr)
                return false;
            buffer = _heap;
        }
        if (napi().getStringUtf8(env, value, buffer, length + 1, &length) != NAPI_OK)
            return false;
        _pointer = buffer;
        return true;
    }

    // Указатель на текст или NULL / A pointer to the text or NULL
    const char* ArgString::get() const noexcept
    {
        return _pointer;
    }
}
