#include "capi/internal/capi_support.hpp"
#include "i18n/i18n.hpp"

#include <string>

namespace
{
    /*
    ** Номер сообщения Си-интерфейса -> идентификатор каталога. Номера
    ** beresta_message стабильны (это ABI для обёрток), а перечисление
    ** i18n::Message внутреннее и может меняться — поэтому нужна таблица.
    **
    ** A C interface message number -> a catalog identifier. beresta_message
    ** numbers are stable (an ABI for the bindings), while i18n::Message is
    ** internal and may change — hence the table.
    */
    const beresta::i18n::Message BINDING_MESSAGES[] = {
        beresta::i18n::Message::BINDING_DOCUMENT_CLOSED,
        beresta::i18n::Message::BINDING_XPATH_CLOSED,
        beresta::i18n::Message::BINDING_SCHEMA_CLOSED,
        beresta::i18n::Message::BINDING_NATIVE_CALL_FAILED,
        beresta::i18n::Message::BINDING_UNKNOWN_NODE_TYPE,
        beresta::i18n::Message::BINDING_INVALID_ARGUMENT
    };

    // Коды языков через запятую: "ru,en" / Language codes separated by commas: "ru,en"
    std::string joinLanguages()
    {
        std::string result;

        for (std::string_view code : beresta::i18n::availableLanguages())
        {
            if (!result.empty())
                result += ',';
            result += code;
        }
        return result;
    }
}

/*
** Смена языка. Ошибка содержит список доступных языков — так пользователь
** обёртки сразу видит, какие коды допустимы.
**
** Changes the language. The error lists the available languages, so a
** binding user sees at once which codes are valid.
*/
extern "C" BERESTA_API int beresta_set_language(const char* code)
{
    BERESTA_CAPI_TRY
        std::string_view wanted = beresta::capi::requireText(code, "code");

        if (!beresta::i18n::setLanguage(wanted))
            throw beresta::capi::InvalidArgumentException(beresta::i18n::format(
                beresta::i18n::Message::LANGUAGE_UNKNOWN, {wanted, beresta_languages()}));
        return BERESTA_OK;
    BERESTA_CAPI_CATCH_STATUS
}

// Код текущего языка: строка из статического каталога, освобождать не нужно
// The current language code: a string from a static catalog, nothing to free
extern "C" BERESTA_API const char* beresta_language(void)
{
    return beresta::i18n::language().data();
}

/*
** Список языков собирается один раз (статическая переменная функции —
** потокобезопасная инициализация) и живёт до конца программы.
**
** The language list is built once (a function-local static — thread-safe
** initialization) and lives until the program ends.
*/
extern "C" BERESTA_API const char* beresta_languages(void)
{
    static const std::string joined = joinLanguages();

    return joined.c_str();
}

// Шаблон сообщения обёртки на текущем языке; неизвестный номер — NULL с ошибкой
// A binding message template in the current language; an unknown number gives NULL with an error
extern "C" BERESTA_API const char* beresta_message_text(int message)
{
    BERESTA_CAPI_TRY
        std::size_t count = sizeof(BINDING_MESSAGES) / sizeof(BINDING_MESSAGES[0]);
        std::size_t index = beresta::capi::checkedIndex(static_cast<std::int64_t>(message) - 1, count);
        const beresta::i18n::Catalog* catalog = beresta::i18n::findCatalog(beresta::i18n::language());

        return beresta::i18n::lookup(*catalog, BINDING_MESSAGES[index]);
    BERESTA_CAPI_CATCH(nullptr)
}
