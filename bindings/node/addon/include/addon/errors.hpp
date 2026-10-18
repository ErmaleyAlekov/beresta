#ifndef BERESTA_NODE_ERRORS_HPP
#define BERESTA_NODE_ERRORS_HPP

#include "addon/napi_abi.hpp"

#include <cstddef>

namespace beresta::node
{
    /*
    ** Данные аддона в одном окружении Node (главный поток или воркер):
    ** ссылка на класс BerestaError из JavaScript. У каждого воркера своё
    ** окружение, поэтому данные хранятся через napi_set_instance_data, а не
    ** в глобальной переменной, — воркеры не мешают друг другу.
    **
    ** The addon data in one Node environment (the main thread or a worker):
    ** a reference to the BerestaError class from JavaScript. Each worker has
    ** its own environment, so the data is kept through napi_set_instance_data,
    ** not in a global variable — the workers do not interfere with each other.
    */
    struct AddonState
    {
        napi_ref errorClass;
    };

    // ---- Состояние окружения (addon_state.cpp) / Environment state ----

    // Функция init(BerestaError) для JavaScript / The init(BerestaError) function for JavaScript
    napi_value initialize(napi_env env, napi_callback_info info) noexcept;

    // Класс ошибки текущего окружения или nullptr / The error class of the current environment or nullptr
    napi_value errorClass(napi_env env) noexcept;

    // ---- Исключения (errors.cpp) / Exceptions ----

    // BerestaError по последней ошибке Си-интерфейса; всегда nullptr
    // A BerestaError for the last C interface error; always nullptr
    napi_value throwLastError(napi_env env) noexcept;

    // TypeError «неверный аргумент №index функции name»; всегда nullptr
    // A TypeError "invalid argument #index of the function name"; always nullptr
    napi_value throwArgumentError(napi_env env, std::size_t index, const char* name, const char* expected) noexcept;
}

#endif
