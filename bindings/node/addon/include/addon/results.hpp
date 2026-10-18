#ifndef BERESTA_NODE_RESULTS_HPP
#define BERESTA_NODE_RESULTS_HPP

#include "addon/napi_abi.hpp"

#include <cstddef>
#include <cstdint>

/*
** Перевод результатов Си-интерфейса в значения JavaScript. Соглашения
** Си-интерфейса одинаковы для всех функций, поэтому и их обработка
** написана один раз (DRY):
**   - дескриптор: NULL с ошибкой — исключение, NULL без ошибки — null,
**     иначе BigInt (адрес сравнивается через ===);
**   - статус: не 0 — исключение BerestaError;
**   - char*: копируется в строку JavaScript и сразу освобождается;
**   - -1 у счётчиков и логических функций — исключение.
** Каждая функция при исключении возвращает nullptr — так переходник
** сообщает Node, что исключение уже брошено.
**
** Converting C interface results into JavaScript values. The C interface
** conventions are the same for all functions, so their handling is written
** once too (DRY):
**   - a handle: NULL with an error is an exception, NULL without an error
**     is null, otherwise a BigInt (the address is compared with ===);
**   - a status: anything but 0 is a BerestaError exception;
**   - char*: copied into a JavaScript string and freed at once;
**   - -1 from counters and boolean functions is an exception.
** On an exception each function returns nullptr — this is how an adapter
** tells Node that the exception is already thrown.
*/
namespace beresta::node
{
    // ---- Дескрипторы и статусы (results_handles.cpp) / Handles and statuses ----
    napi_value makeHandle(napi_env env, void* handle) noexcept;
    napi_value makeStatus(napi_env env, int status) noexcept;
    napi_value makeOut(napi_env env, int status, void* handle) noexcept;
    napi_value makeNull(napi_env env) noexcept;
    napi_value makeUndefined(napi_env env) noexcept;

    // ---- Строки (results_strings.cpp) / Strings ----
    napi_value makeOwnedString(napi_env env, char* text) noexcept;
    napi_value makeBorrowedString(napi_env env, const char* text) noexcept;

    // ---- Числа и логические значения (results_numbers.cpp) / Numbers and booleans ----
    napi_value makeCount(napi_env env, std::int64_t value) noexcept;
    napi_value makeFlag(napi_env env, int value) noexcept;
    napi_value makeInteger(napi_env env, int value) noexcept;
    napi_value makeNumber(napi_env env, double value) noexcept;
}

#endif
