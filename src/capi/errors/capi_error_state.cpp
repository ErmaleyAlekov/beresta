#include "capi/internal/capi_support.hpp"

namespace beresta::capi
{
    /*
    ** Хранилище последней ошибки — своё у каждого потока (thread_local).
    ** Так реализован errno в C и GetLastError() в WinAPI: вызовы из разных
    ** потоков не затирают ошибки друг друга, и не нужен ни один мьютекс.
    **
    ** The storage of the last error — each thread has its own (thread_local).
    ** This is how errno in C and GetLastError() in WinAPI work: calls from different
    ** threads do not overwrite each other's errors, and no mutex is needed.
    */
    ErrorState& errorState() noexcept
    {
        thread_local ErrorState state = {BERESTA_OK, {'\0'}};

        return state;
    }

    // Сброс перед каждым вызовом: код ошибки всегда относится к ПОСЛЕДНЕЙ функции
    // Reset before every call: the error code always refers to the LAST function
    void beginCall() noexcept
    {
        ErrorState& state = errorState();

        state.code = BERESTA_OK;
        state.message[0] = '\0';
    }
}

// Код последней ошибки текущего потока
// The last error code of the current thread
extern "C" BERESTA_API int beresta_last_error_code(void)
{
    return beresta::capi::errorState().code;
}

// Сообщение последней ошибки: указатель живёт до следующего вызова в потоке
// The last error message: the pointer lives until the next call in the thread
extern "C" BERESTA_API const char* beresta_last_error_message(void)
{
    return beresta::capi::errorState().message;
}
