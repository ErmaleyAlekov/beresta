#include "addon/errors.hpp"
#include "capi/beresta_c.h"
#include "i18n/i18n.hpp"

#include <string>

namespace beresta::node
{
    // Аргументы конструктора BerestaError(status, code, message) / The BerestaError(status, code, message) constructor arguments
    static bool errorArguments(napi_env env, int status, const std::string& message, napi_value* out) noexcept
    {
        const char* name = beresta_status_name(status);

        return napi().createDouble(env, static_cast<double>(status), &out[0]) == NAPI_OK
            && napi().createStringUtf8(env, name, NAPI_AUTO_LENGTH, &out[1]) == NAPI_OK
            && napi().createStringUtf8(env, message.c_str(), message.size(), &out[2]) == NAPI_OK;
    }

    /*
    ** Исключение по последней ошибке потока. Текст копируется сразу: строка
    ** beresta_last_error_message заимствованная и живёт до следующего вызова.
    ** Без зарегистрированного класса (init не вызван) бросается обычный
    ** Error с кодом — так ошибка не теряется ни при каком порядке загрузки.
    **
    ** An exception for the last error of the thread. The text is copied at once:
    ** the beresta_last_error_message string is borrowed and lives until the next call.
    ** Without a registered class (init was not called) an ordinary Error with
    ** a code is thrown — so the error is never lost, whatever the load order.
    */
    napi_value throwLastError(napi_env env) noexcept
    {
        int status = beresta_last_error_code();
        napi_value constructor = errorClass(env);
        napi_value arguments[3] = {nullptr, nullptr, nullptr};
        napi_value error = nullptr;

        try
        {
            std::string message(beresta_last_error_message());

            if (constructor != nullptr && errorArguments(env, status, message, arguments)
                && napi().newInstance(env, constructor, 3, arguments, &error) == NAPI_OK)
                napi().throwValue(env, error);
            else
                napi().throwError(env, beresta_status_name(status), message.c_str());
        }
        catch (...)
        {
            napi().throwError(env, beresta_status_name(status), beresta_last_error_message());
        }
        return nullptr;
    }

    /*
    ** TypeError о неверном аргументе. Текст берётся из каталога сообщений на
    ** текущем языке, как и все остальные сообщения библиотеки.
    **
    ** A TypeError about an invalid argument. The text is taken from the message
    ** catalog in the current language, like all other library messages.
    */
    napi_value throwArgumentError(napi_env env, std::size_t index, const char* name, const char* expected) noexcept
    {
        try
        {
            std::string message = i18n::format(i18n::Message::BINDING_INVALID_ARGUMENT,
                {std::to_string(index), name, expected});

            napi().throwTypeError(env, "INVALID_ARGUMENT", message.c_str());
        }
        catch (...)
        {
            napi().throwTypeError(env, "INVALID_ARGUMENT", name);
        }
        return nullptr;
    }
}
