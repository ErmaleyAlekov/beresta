#include "capi/internal/capi_support.hpp"
#include "exception/dom_exception.hpp"

#include <cstring>
#include <new>

namespace beresta::capi
{
    /*
    ** Перевод исключения в код состояния. Порядок проверок важен:
    ** сначала собственные исключения библиотеки (у них есть точный код),
    ** затем нехватка памяти, всё прочее — «неизвестная ошибка».
    ** Код DomErrorCode сдвигается на единицу: 0 зарезервирован за BERESTA_OK.
    */
    static int statusOf(const std::exception& error) noexcept
    {
        const DomException* domError = dynamic_cast<const DomException*>(&error);

        if (domError != nullptr)
            return static_cast<int>(domError->getCode()) + 1;
        if (dynamic_cast<const InvalidArgumentException*>(&error) != nullptr)
            return BERESTA_INVALID_ARGUMENT;
        if (dynamic_cast<const std::bad_alloc*>(&error) != nullptr)
            return BERESTA_OUT_OF_MEMORY;
        return BERESTA_UNKNOWN_ERROR;
    }

    /*
    ** Копирование сообщения в буфер фиксированного размера. Слишком длинное
    ** сообщение обрезается по границе символа UTF-8: байты вида 10xxxxxx —
    ** продолжение многобайтового символа, на них резать нельзя, иначе
    ** обёртка получила бы некорректную строку UTF-8.
    */
    static void copyMessage(const char* text, char* buffer) noexcept
    {
        std::size_t length = std::strlen(text);

        if (length >= BERESTA_CAPI_MESSAGE_CAPACITY)
        {
            length = BERESTA_CAPI_MESSAGE_CAPACITY - 1;
            while (length > 0 && (static_cast<unsigned char>(text[length]) & 0xC0) == 0x80)
                --length;
        }
        std::memcpy(buffer, text, length);
        buffer[length] = '\0';
    }

    /*
    ** Текст для внешнего мира. У исключений библиотеки берётся сообщение
    ** БЕЗ префикса "[КОД]" из what(): код и так доступен отдельно через
    ** beresta_last_error_code, а обёртки сами решают, как его показать.
    */
    static const char* messageOf(const std::exception& error) noexcept
    {
        const DomException* domError = dynamic_cast<const DomException*>(&error);

        if (domError != nullptr)
            return domError->getMessage().c_str();
        return error.what();
    }

    // Запись ошибки: ни одной операции, способной бросить исключение
    int recordException(const std::exception& error) noexcept
    {
        ErrorState& state = errorState();

        state.code = statusOf(error);
        copyMessage(messageOf(error), state.message);
        return state.code;
    }

    // Исключение не из иерархии std::exception: текст неизвестен
    int recordUnknown() noexcept
    {
        ErrorState& state = errorState();

        state.code = BERESTA_UNKNOWN_ERROR;
        copyMessage("непредвиденное исключение", state.message);
        return state.code;
    }
}
