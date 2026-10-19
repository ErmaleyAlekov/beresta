#ifndef BERESTA_I18N_HPP
#define BERESTA_I18N_HPP

#include "i18n/message.hpp"

#include <cstddef>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

/*
** Язык сообщений об ошибках по умолчанию задаётся при сборке:
**   make ERRORS_LANG=en      или      cmake -DBERESTA_ERRORS_LANG=en
** Значение — код каталога (имя файла src/i18n/catalogs/messages_<код>.cpp).
** Во время работы язык можно сменить функцией beresta::i18n::setLanguage.
**
** The default language of error messages is chosen at build time:
**   make ERRORS_LANG=en      or      cmake -DBERESTA_ERRORS_LANG=en
** The value is a catalog code (file name src/i18n/catalogs/messages_<code>.cpp).
** The language can be changed at run time with beresta::i18n::setLanguage.
*/
#ifndef BERESTA_ERRORS_LANG
    #define BERESTA_ERRORS_LANG ru
#endif

// Превращение имени-лексемы в строку: ru -> "ru" (без кавычек в командной строке сборки)
// Turns a token into a string: ru -> "ru" (no quotes needed on the build command line)
#define BERESTA_I18N_STRINGIZE_TOKEN(token) #token
#define BERESTA_I18N_STRINGIZE(token) BERESTA_I18N_STRINGIZE_TOKEN(token)

namespace beresta::i18n
{
    /*
    ** Одна запись каталога: идентификатор сообщения и его шаблон. В шаблоне
    ** {0}, {1}, ... — места для аргументов, которые подставляет format().
    ** Запись хранит идентификатор явно, поэтому порядок строк в таблице
    ** языка не важен, а пропущенный перевод не сдвигает остальные.
    **
    ** One catalog entry: a message identifier and its template. {0}, {1}, ...
    ** in the template are placeholders for the arguments that format()
    ** substitutes. The entry stores its identifier explicitly, so the order of
    ** rows in a language table does not matter and a missing translation
    ** does not shift the others.
    */
    struct MessageText
    {
        Message id;
        const char* text;
    };

    /*
    ** Каталог одного языка (паттерн Strategy: язык — сменяемая стратегия
    ** получения текста). code — код для выбора языка ("ru", "en", "de"...),
    ** name — самоназвание языка для списков выбора.
    **
    ** A catalog of one language (the Strategy pattern: a language is an
    ** interchangeable strategy for obtaining text). code selects the language
    ** ("ru", "en", "de"...), name is the language's own name for pick lists.
    */
    struct Catalog
    {
        const char* code;
        const char* name;
        const MessageText* entries;
        std::size_t size;
    };

    /*
    ** Текст сообщения на текущем языке с подставленными аргументами.
    ** Если в каталоге языка нет перевода, берётся английский текст: новое
    ** сообщение никогда не превратится в пустую строку.
    **
    ** The message text in the current language with arguments substituted.
    ** If the language catalog has no translation, the English text is used:
    ** a new message never turns into an empty string.
    */
    std::string format(Message id, std::initializer_list<std::string_view> arguments = {});

    /*
    ** Смена языка для всех потоков. false — каталога с таким кодом нет,
    ** язык не меняется. Потокобезопасно: текущий язык хранится атомарно.
    **
    ** Changes the language for all threads. false means there is no catalog
    ** with this code and the language stays the same. Thread-safe: the current
    ** language is stored atomically.
    */
    bool setLanguage(std::string_view code);

    // Код текущего языка, например "ru"
    // The current language code, for example "ru"
    std::string_view language() noexcept;

    // Коды всех встроенных каталогов в порядке реестра
    // Codes of all built-in catalogs in registry order
    std::vector<std::string_view> availableLanguages();

    // ---- Внутреннее устройство (i18n_registry.cpp) / Internals ----

    /*
    ** Все каталоги библиотеки. Чтобы добавить язык: создать файл
    ** src/i18n/catalogs/messages_<код>.cpp с таблицей и добавить одну
    ** строку в реестр в i18n_registry.cpp (см. docs/guide.md, раздел «Языки сообщений»).
    **
    ** All library catalogs. To add a language: create the file
    ** src/i18n/catalogs/messages_<code>.cpp with a table and add one line to
    ** the registry in i18n_registry.cpp (see docs/guide.en.md, the "Message languages" section).
    */
    const std::vector<const Catalog*>& catalogs();

    // Шаблон сообщения в каталоге (с запасным английским); nullptr не бывает
    // The message template in a catalog (with English fallback); never nullptr
    const char* lookup(const Catalog& catalog, Message id);

    // Каталог по коду или nullptr
    // A catalog by code, or nullptr
    const Catalog* findCatalog(std::string_view code) noexcept;

    // Встроенные каталоги (src/i18n/catalogs/)
    // Built-in catalogs (src/i18n/catalogs/)
    extern const Catalog RUSSIAN_CATALOG;
    extern const Catalog ENGLISH_CATALOG;
}

#endif
