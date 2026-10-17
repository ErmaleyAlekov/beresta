#include "capi/internal/capi_support.hpp"

namespace beresta::capi
{
    /*
    ** Хранилище последней ошибки — своё у каждого потока (thread_local).
    ** Так реализован errno в C и GetLastError() в WinAPI: вызовы из разных
    ** потоков не затирают ошибки друг друга, и не нужен ни один мьютекс.
    */
    ErrorState& errorState() noexcept
    {
        thread_local ErrorState state = {BERESTA_OK, {'\0'}};

        return state;
    }

    // Сброс перед каждым вызовом: код ошибки всегда относится к ПОСЛЕДНЕЙ функции
    void beginCall() noexcept
    {
        ErrorState& state = errorState();

        state.code = BERESTA_OK;
        state.message[0] = '\0';
    }
}

// Код последней ошибки текущего потока
extern "C" BERESTA_API int beresta_last_error_code(void)
{
    return beresta::capi::errorState().code;
}

// Сообщение последней ошибки: указатель живёт до следующего вызова в потоке
extern "C" BERESTA_API const char* beresta_last_error_message(void)
{
    return beresta::capi::errorState().message;
}
