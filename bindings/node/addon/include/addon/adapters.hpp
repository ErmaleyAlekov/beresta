#ifndef BERESTA_NODE_ADAPTERS_HPP
#define BERESTA_NODE_ADAPTERS_HPP

#include "addon/napi_abi.hpp"

#include <cstdint>

/*
** Переходники (паттерн Adapter) между вызовом из JavaScript и функцией
** Си-интерфейса. Переходник знает СИГНАТУРУ, но не конкретную функцию:
** он читает аргументы из кадра, вызывает функцию из описания Binding и
** превращает результат в значение JavaScript. Поэтому 71 функцию
** Си-интерфейса обслуживают всего 31 переходник.
** Имя переходника читается как «что возвращает» + «что принимает»:
** adaptOwnedOfHandleIndex — char* (владение у вызывающего) от (дескриптор, индекс).
**
** Adapters (the Adapter pattern) between a call from JavaScript and a C
** interface function. An adapter knows the SIGNATURE, not a specific function:
** it reads the arguments from the frame, calls the function from the Binding
** description and turns the result into a JavaScript value. That is why the
** 71 C interface functions are served by only 31 adapters.
** An adapter name reads as "what it returns" + "what it takes":
** adaptOwnedOfHandleIndex — a char* (owned by the caller) of (handle, index).
*/
namespace beresta::node
{
    // ---- Сигнатуры функций Си-интерфейса / C interface function signatures ----
    using BorrowedOfNothing = const char* (*)();
    using BorrowedOfInt = const char* (*)(int);
    using StatusOfString = int (*)(const char*);
    using OutOfString = int (*)(const char*, void**);
    using VoidOfHandle = void (*)(void*);
    using OutOfHandle = int (*)(void*, void**);
    using HandleOfHandle = void* (*)(void*);
    using HandleOfHandleString = void* (*)(void*, const char*);
    using HandleOfHandleIndex = void* (*)(void*, std::int64_t);
    using HandleOfHandleStrings = void* (*)(void*, const char*, const char*);
    using StatusOfHandle = int (*)(void*);
    using StatusOfHandles = int (*)(void*, void*);
    using StatusOfHandleString = int (*)(void*, const char*);
    using StatusOfHandleStrings = int (*)(void*, const char*, const char*);
    using StatusOfHandleStringNumber = int (*)(void*, const char*, double);
    using OwnedOfHandle = char* (*)(void*);
    using OwnedOfHandleFlags = char* (*)(void*, std::uint32_t);
    using OwnedOfHandleString = char* (*)(void*, const char*);
    using OwnedOfHandleIndex = char* (*)(void*, std::int64_t);
    using OwnedOfHandles = char* (*)(void*, void*);
    using CountOfHandle = std::int64_t (*)(void*);
    using CountOfHandleIndex = std::int64_t (*)(void*, std::int64_t);
    using FlagOfHandles = int (*)(void*, void*);
    using NumberOfHandles = int (*)(void*, void*, double*);
    using OutOfHandles = int (*)(void*, void*, void**);
    using ParseBytes = int (*)(const void*, std::int64_t, std::uint32_t, void*, void**);
    using ParseFile = int (*)(const char*, std::uint32_t, void*, void**);
    using SaveDocument = int (*)(void*, const char*, std::uint32_t, const char*);
    using LoadSchema = int (*)(const void*, std::int64_t, void**);

    // ---- Библиотека, языки, создание и освобождение (adapters_text.cpp) ----
    // ---- Library, languages, creation and freeing (adapters_text.cpp) ----
    napi_value adaptBorrowed(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptBorrowedOfInt(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptStatusOfString(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptOutOfString(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptVoidOfHandle(napi_env env, napi_callback_info info) noexcept;

    // ---- Дескрипторы (adapters_handles.cpp) / Handles ----
    napi_value adaptOutOfHandle(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptHandleOfHandle(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptHandleOfHandleString(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptHandleOfHandleIndex(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptHandleOfHandleStrings(napi_env env, napi_callback_info info) noexcept;

    // ---- Статусы (adapters_status.cpp) / Statuses ----
    napi_value adaptStatusOfHandle(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptIntegerOfHandle(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptStatusOfHandleString(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptStatusOfHandleStrings(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptStatusOfHandleStringNumber(napi_env env, napi_callback_info info) noexcept;

    // ---- Статусы от двух дескрипторов (adapters_status_pairs.cpp) / Statuses of two handles ----
    napi_value adaptStatusOfHandles(napi_env env, napi_callback_info info) noexcept;

    // ---- Строки (adapters_strings.cpp) / Strings ----
    napi_value adaptOwnedOfHandle(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptOwnedOfHandleFlags(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptOwnedOfHandleString(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptOwnedOfHandleIndex(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptOwnedOfHandles(napi_env env, napi_callback_info info) noexcept;

    // ---- Числа и логические значения (adapters_numbers.cpp) / Numbers and booleans ----
    napi_value adaptCountOfHandle(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptCountOfHandleIndex(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptFlagOfHandleString(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptFlagOfHandles(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptNumberOfHandles(napi_env env, napi_callback_info info) noexcept;

    // ---- Разбор, запись и составные вызовы (adapters_special.cpp) ----
    // ---- Parsing, saving and compound calls (adapters_special.cpp) ----
    napi_value adaptOutOfHandles(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptParse(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptParseFile(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptSave(napi_env env, napi_callback_info info) noexcept;
    napi_value adaptLoadSchema(napi_env env, napi_callback_info info) noexcept;
}

#endif
